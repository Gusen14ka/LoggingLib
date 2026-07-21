#include <vector>
#include <string>

#include <gtest/gtest.h>
#include "logger/logger.hpp"
#include "logger/strategies/log_strategy.hpp"
#include "logger/time/time.hpp"


namespace {
// Мок-стратегия: не пишет никуда реально, просто запоминает каждый вызов write()
// и позволяет тесту заранее настроить, должен ли следующий write() "провалиться".
class MockLogStrategy : public ILogStrategy {
public:
    bool write(std::string const& message, std::string& err) override {
        if (should_fail_) {
            err = fail_message_;
            return false;
        }
        written_messages_.push_back(message);
        return true;
    }

    std::vector<std::string> const& written_messages() const { return written_messages_; }

    void set_should_fail(bool fail, std::string message) {
        should_fail_ = fail;
        fail_message_ = std::move(message);
    }

private:
    std::vector<std::string> written_messages_;
    bool should_fail_ = false;
    std::string fail_message_;
};

// Хелпер: создаёт Logger с mock-стратегией и отдаёт наружу сырой указатель на мок,
// чтобы тест мог инспектировать записанные сообщения (Logger владеет стратегией через unique_ptr,
// но нам нужен доступ к тому же объекту после передачи владения).
std::pair<std::unique_ptr<Logger>, MockLogStrategy*> make_logger_with_mock(LogLevel default_level) {
    auto mock = std::make_unique<MockLogStrategy>();
    MockLogStrategy* mock_ptr = mock.get();
    auto logger = Logger::create(std::move(mock), default_level);
    return {std::move(logger), mock_ptr};
}

}  // namespace

TEST(LoggerTest, CreateWithNullStrategyReturnsNullptr) {
    std::unique_ptr<ILogStrategy> null_strategy = nullptr;
    auto logger = Logger::create(std::move(null_strategy), LogLevel::INFO);
    EXPECT_EQ(logger, nullptr);
}

TEST(LoggerTest, LogsMessageAtOrAboveDefaultLevel) {
    auto [logger, mock] = make_logger_with_mock(LogLevel::INFO);
    ASSERT_NE(logger, nullptr);

    logger->log("hello", LogLevel::INFO);
    logger->log("world", LogLevel::ERROR);

    ASSERT_EQ(mock->written_messages().size(), 2);
}

TEST(LoggerTest, FiltersMessagesBelowDefaultLevel) {
    auto [logger, mock] = make_logger_with_mock(LogLevel::WARNING);
    ASSERT_NE(logger, nullptr);

    logger->log("should be dropped", LogLevel::DEBUG);
    logger->log("also dropped", LogLevel::INFO);
    logger->log("kept", LogLevel::WARNING);
    logger->log("kept too", LogLevel::ERROR);

    ASSERT_EQ(mock->written_messages().size(), 2);
}

TEST(LoggerTest, LogWithoutExplicitLevelUsesDefaultLevel) {
    auto [logger, mock] = make_logger_with_mock(LogLevel::ERROR);
    ASSERT_NE(logger, nullptr);

    logger->log("no level given");  // должно использовать default_level_ = ERROR внутри

    ASSERT_EQ(mock->written_messages().size(), 1);
    // проверяем, что в отформатированной строке действительно ERROR, не какой-то другой уровень
    ASSERT_EQ(mock->written_messages()[0].substr(0, 7), "[ERROR]");
}

TEST(LoggerTest, ChangeDefaultLevelAffectsSubsequentCalls) {
    auto [logger, mock] = make_logger_with_mock(LogLevel::DEBUG);
    ASSERT_NE(logger, nullptr);

    logger->log("visible under DEBUG default");  // проходит, default = DEBUG
    logger->change_default_level(LogLevel::ERROR);
    logger->log("should be dropped now");         // default стал ERROR, вызов без явного уровня использует его же

    // ВАЖНО: если change_default_level меняет и порог фильтрации, и то, что подставляется
    // в log() без явного уровня — второе сообщение пройдёт САМО фильтрацию (ERROR >= ERROR),
    // но проверяет именно то, что подставленный уровень актуализировался
    ASSERT_EQ(mock->written_messages().size(), 2);
    ASSERT_EQ(mock->written_messages()[1].substr(0, 7), "[ERROR]");
}

TEST(LoggerTest, FormattedMessageContainsLevelTimestampAndText) {
    auto [logger, mock] = make_logger_with_mock(LogLevel::DEBUG);
    ASSERT_NE(logger, nullptr);

    logger->log("distinctive text 12345", LogLevel::WARNING);

    // Воспользуемся отдельно тестируемой функцией (см. test_timestamps.cpp)
    auto timestamp = current_timestamp();
    auto timestamp_without_ms = timestamp.substr(0, timestamp.length() - 3);

    ASSERT_EQ(mock->written_messages().size(), 1);
    auto const& line = mock->written_messages()[0];
    EXPECT_NE(line.find("[WARNING]"), std::string::npos);
    EXPECT_NE(line.find("[distinctive text 12345]"), std::string::npos);
    EXPECT_NE(line.find(timestamp_without_ms), std::string::npos);
}

TEST(LoggerTest, WriteFailureDoesNotCrashAndDoesNotThrow) {
    auto [logger, mock] = make_logger_with_mock(LogLevel::DEBUG);
    ASSERT_NE(logger, nullptr);

    mock->set_should_fail(true, "simulated disk error");

    // исключения не должно быть, но будет "побочный" вывод в консоль
    EXPECT_NO_THROW(logger->log("this write will fail", LogLevel::ERROR));
    // сообщение не должно было "записаться" (мок его не сохранил при should_fail)
    EXPECT_TRUE(mock->written_messages().empty());
}

TEST(LoggerTest, RecoversAfterWriteFailureStops) {
    auto [logger, mock] = make_logger_with_mock(LogLevel::DEBUG);
    ASSERT_NE(logger, nullptr);

    mock->set_should_fail(true, "simulated disk error");
    logger->log("fails", LogLevel::ERROR);

    mock->set_should_fail(false, "simulated disk error");
    logger->log("succeeds", LogLevel::ERROR);

    ASSERT_EQ(mock->written_messages().size(), 1);
    // Ранее полученная ошибка не мешает продолжить логирование
    EXPECT_NE(mock->written_messages()[0].find("succeeds"), std::string::npos);
}