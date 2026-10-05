#include <cassert>
#include <string>
#include <CANBridge/Types.h>
#include <internal/ConfiguredBus.hpp>
#include <CANBridge/Examples.h>
using namespace canbridge;
struct TestConfig { std::uint32_t bitrate = 1000000; bool listenOnly = false; };
struct Fake : Controller {
    static bool failCleanup, mismatch, silent, accessFailure, operationFailure;
    static unsigned receives;
    Frame frame;
    bool pending = false;
    Result begin(const CommonConfig &) override { return Result::Ok; }
    Result beginDiagnostic(const CommonConfig &c, DiagnosticReport &r) override {
        assert(c.selfTest && !c.listenOnly);
        r.controllerAccess = {accessFailure ? TestStatus::Failed : TestStatus::Passed, accessFailure ? Result::DriverError : Result::Ok};
        return accessFailure || operationFailure ? Result::DriverError : Result::Ok;
    }
    Result end() override { return failCleanup ? Result::DriverError : Result::Ok; }
    Result send(const Frame &f) override { frame = f; pending = true; return Result::Ok; }
    Result receive(Frame &f) override {
        ++receives;
        if (!pending || silent) return Result::Empty;
        f = frame; pending = false; if (mismatch) f.id ^= 1; return Result::Ok;
    }
    Result pollHealth(Health &h) override { h = Health{}; return Result::Ok; }
};
bool Fake::failCleanup = false, Fake::mismatch = false, Fake::silent = false, Fake::accessFailure = false, Fake::operationFailure = false;
unsigned Fake::receives = 0;
struct Factory {
    using Backend = Fake; using Configuration = TestConfig;
    static Result check(const TestConfig &c) { return c.bitrate ? Result::Ok : Result::MissingBitrate; }
    static Fake *create(const TestConfig &) { return new Fake; }
};
struct BusyBus {
    unsigned receiveCalls = 0, sends = 0;
    Frame received;
    Frame accepted;
    Result begin(const TestConfig &) { return Result::Ok; }
    Result diagnose(const TestConfig &, DiagnosticReport &) { return Result::Unsupported; }
    Result send(const Frame &f) { ++sends; accepted = f; return sends <= 3 ? Result::Busy : Result::Ok; }
    Result receive(Frame &f) { ++receiveCalls; f = received; return receiveCalls <= 1 ? Result::Ok : Result::Empty; }
    Result pollHealth(Health &h) { h = Health{}; return Result::Ok; }
};
int main() {
    Frame f;
    assert(detail::parseFrame("S 123#112233AABB", f));
    assert(f.id == 0x123 && f.length == 5 && f.data[4] == 0xBB);
    char text[32]; detail::formatFrame(f, text); assert(std::string(text) == "S 123#112233AABB");
    assert(detail::parseFrame("E 001ABCDE#01020304", f));
    detail::formatFrame(f, text); assert(std::string(text) == "E 001ABCDE#01020304");
    assert(detail::parseFrame("S 123#R8", f)); assert(f.remote && f.length == 8);
    detail::formatFrame(f, text); assert(std::string(text) == "S 123#R8");
    assert(detail::parseFrame("S 123#", f)); assert(f.length == 0);
    assert(detail::parseFrame("E 1FFFFFFF#0011223344556677", f));
    for (const char *bad : {"", "S", "S 800#", "E 20000000#", "S 123#1", "S 123#001122334455667788", "S 123#R9", "S 123#R", "S 123#R8x", "S #", "S 123#GG"}) {
        f.id = 77; assert(!detail::parseFrame(bad, f)); assert(f.id == 77);
    }
    ConfiguredBus<Factory> bus; TestConfig c; DiagnosticReport report;
    c.listenOnly = true;
    assert(bus.diagnose(c, report) == Result::Ok);
    assert(report.framesChecked == 4 && report.cleanup.status == TestStatus::Passed);
    assert(bus.begin(c) == Result::Ok);
    assert(bus.diagnose(c, report) == Result::AlreadyStarted);
    assert(bus.end() == Result::Ok);
    Fake::accessFailure = true;
    assert(bus.diagnose(c, report) == Result::DriverError);
    assert(report.controllerAccess.status == TestStatus::Failed);
    assert(report.controllerOperation.status == TestStatus::NotRun && report.internalLoopback.status == TestStatus::NotRun);
    Fake::accessFailure = false; Fake::operationFailure = true;
    assert(bus.diagnose(c, report) == Result::DriverError);
    assert(report.controllerOperation.status == TestStatus::Failed && report.internalLoopback.status == TestStatus::NotRun);
    Fake::operationFailure = false;
    Fake::mismatch = true;
    assert(bus.diagnose(c, report) == Result::DataMismatch);
    assert(report.internalLoopback.status == TestStatus::Failed && report.framesChecked == 0);
    Fake::mismatch = false; Fake::silent = true;
    testMillis = 0xfffffff0U;
    assert(bus.diagnose(c, report) == Result::Timeout);
    Fake::silent = false; Fake::failCleanup = true;
    assert(bus.diagnose(c, report) == Result::DriverError);
    assert(report.cleanup.status == TestStatus::Failed);
    assert(bus.begin(c) == Result::AlreadyStarted);
    Fake::failCleanup = false; assert(bus.end() == Result::Ok);
    c.bitrate = 0; assert(bus.diagnose(c, report) == Result::MissingBitrate);
    assert(report.controllerAccess.status == TestStatus::NotRun);
    c.bitrate = 1000000;
    examples::Runner<ConfiguredBus<Factory>> runner(bus, examples::Mode::SerialInput);
    runner.begin(c);
    for (char ch : std::string("S 123#1122\n")) Serial.input.push_back(ch);
    Serial.capacity = 0; runner.loop(); runner.loop();
    const auto before = Fake::receives;
    runner.loop(); assert(Fake::receives > before); // blocked serial must not block CAN.
    Serial.capacity = 64;
    for (unsigned i = 0; i < 20; ++i) runner.loop();
    assert(Serial.output.find("TX accepted S 123#1122") != std::string::npos);
    assert(Serial.output.find("RX S 123#1122") != std::string::npos);
    assert(bus.end() == Result::Ok);
    BusyBus busy; busy.received.id = 0x456;
    examples::Runner<BusyBus> inputRunner(busy, examples::Mode::SerialInput);
    inputRunner.begin(c); Serial.output.clear();
    for (char ch : std::string("S 123#1122\n")) Serial.input.push_back(ch);
    inputRunner.loop();
    for (char ch : std::string("S 789#33\n")) Serial.input.push_back(ch);
    inputRunner.loop(); inputRunner.loop(); inputRunner.loop();
    assert(busy.accepted.id == 0x123 && busy.accepted.data[0] == 0x11 && busy.receiveCalls >= 4);
    for (unsigned i = 0; i < 20; ++i) inputRunner.loop();
    assert(Serial.output.find("INPUT rejected: transmission pending") != std::string::npos);
    assert(Serial.output.find("RX S 456#") != std::string::npos);
}
