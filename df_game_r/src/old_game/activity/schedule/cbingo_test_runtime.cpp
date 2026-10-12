#include "old_game/activity/schedule/cbingo_test_runtime.h"

#include <boost/date_time/posix_time/posix_time.hpp>

#include <stdint.h>

// IDA_EA: 0x80CAECC
// IDA_NAME: _ZN10CBingoTestC2Ev
// DEMANGLED: CBingoTest::CBingoTest(void)
CBingoTest::CBingoTest() : CBingo() {}

// IDA_EA: 0x80CAEE0
// IDA_NAME: _ZN10CBingoTestD2Ev
// DEMANGLED: CBingoTest::~CBingoTest()
CBingoTest::~CBingoTest() {}

// IDA_EA: 0x80CAEF4
// IDA_NAME: _ZN10CBingoTest3runEv
// DEMANGLED: CBingoTest::run(void)
int CBingoTest::run() {
    getToday();
    resetBoard();
    resetQuiz();
    calBoradMatchLine();
    calNewValue();
    return getBingoRandomVaule();
}

// IDA_EA: 0x80CAF3E
// IDA_NAME: _ZN10CBingoTest17getTimeFromStringESs
// DEMANGLED: CBingoTest::getTimeFromString(std::string)
std::time_t CBingoTest::getTimeFromString(std::string value) {
    const boost::posix_time::ptime parsed =
        boost::posix_time::time_from_string(value);
    struct tm converted = boost::posix_time::to_tm(parsed);
    return std::mktime(&converted);
}

// IDA_EA: 0x80CAF7C
// IDA_NAME: _ZN10CBingoTest8getTodayEv
// DEMANGLED: CBingoTest::getToday(void)
void CBingoTest::getToday() {
    // This is an old in-process test routine.  Its original body only creates
    // and replaces this local timestamp string; it does not mutate CBingo.
    std::string timestamp;
    timestamp = "2012-07-19 05:00:00";
    timestamp = "2012-07-19 06:00:00";
    timestamp = "2012-07-20 12:00:00";
    timestamp = "2012-07-31 11:00:00";
    timestamp = "2012-08-01 05:00:00";
    timestamp = "2012-08-01 07:00:00";
}

// IDA_EA: 0x80CB030
// IDA_NAME: _ZN10CBingoTest10resetBoardEv
// DEMANGLED: CBingoTest::resetBoard(void)
void CBingoTest::resetBoard() {}

// IDA_EA: 0x80CB036
// IDA_NAME: _ZN10CBingoTest9resetQuizEv
// DEMANGLED: CBingoTest::resetQuiz(void)
void CBingoTest::resetQuiz() {}

// IDA_EA: 0x80CB03C
// IDA_NAME: _ZN10CBingoTest17calBoradMatchLineEv
// DEMANGLED: CBingoTest::calBoradMatchLine(void)
void CBingoTest::calBoradMatchLine() {
    // The old test helper makes each write to the same dead stack slot.
    volatile uint32_t test_bits = 0x0010843Fu;
    test_bits = 0x014610C7u;
    test_bits = 0x001F9110u;
    test_bits = 0x01576DD5u;
    test_bits = 0x01577DD5u;
    test_bits = 0x01FFFFFFu;
}

// IDA_EA: 0x80CB06E
// IDA_NAME: _ZN10CBingoTest11calNewValueEv
// DEMANGLED: CBingoTest::calNewValue(void)
void CBingoTest::calNewValue() {
    volatile uint32_t test_bits = 0;
}

// IDA_EA: 0x80CB07E
// IDA_NAME: _ZN10CBingoTest19getBingoRandomVauleEv
// DEMANGLED: CBingoTest::getBingoRandomVaule(void)
bool CBingoTest::getBingoRandomVaule() {
    for (int index = 0; index <= 99; ++index) {
        CBingo::getBingoRandomVaule(10);
    }
    for (int index = 0; index <= 99; ++index) {
        CBingo::getBingoRandomVaule(20);
    }
    for (int index = 0; index <= 99; ++index) {
        CBingo::getBingoRandomVaule(30);
    }
    for (int index = 0; index <= 99; ++index) {
        CBingo::getBingoRandomVaule(40);
    }
    for (int index = 0; index <= 99; ++index) {
        CBingo::getBingoRandomVaule(50);
    }
    return false;
}
