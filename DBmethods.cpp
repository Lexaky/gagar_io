#include <mysql_connection.h>
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>
// получение последней записи из БД
// запрос по scoremass с порядком по дате убывающий и времени убывающему, lim даст одну верхнюю нужную запись
int getLastSavedScoreFromDB() {
    int lastScore;

    try {
        sql::Driver* driver = get_driver_instance();
        std::unique_ptr<sql::Connection> con(driver->connect("tcp://127.0.0.1:3306", "root", "12345"));
        con->setSchema("duma");

        std::string query = "SELECT SCOREMASS FROM SCORES ORDER BY date DESC, time DESC LIMIT 1;";

        std::unique_ptr<sql::Statement> stmt(con->createStatement());
        std::unique_ptr<sql::ResultSet> res(stmt->executeQuery(query));

        if (res->next()) {
            lastScore = res->getInt("SCOREMASS");
        }

        std::cout << "Last saved score from database: " << lastScore << std::endl;
    }
    catch (sql::SQLException& e) {
        std::cerr << "Error fetching last saved score from database: " << e.what() << std::endl;
    }

    return lastScore;
}
// вытаскивание большего значения из бд в таблице SCORES
int getMaxScore() {
    int maxScore = 10;
    try {
        sql::Driver* driver = get_driver_instance();

        std::unique_ptr<sql::Connection> con(driver->connect("tcp://127.0.0.1:3306", "root", "12345"));
        con->setSchema("duma");

        std::unique_ptr<sql::Statement> stmt(con->createStatement());
        std::unique_ptr<sql::ResultSet> res(stmt->executeQuery("SELECT MAX(scoremass) AS max_score FROM SCORES"));

        if (res->next()) {
            maxScore = res->getInt("max_score");
        }

        std::cout << "Max score from database: " << maxScore << std::endl;
    }
    catch (sql::SQLException& e) {
        std::cerr << "Error retrieving max score from database: " << e.what() << std::endl;
    }

    return maxScore;
}

// Процедура для сохранения результата в базу данных
void saveScoreToDB(float score) {
    try {
        sql::Driver* driver = get_driver_instance();
        std::unique_ptr<sql::Connection> con(driver->connect("tcp://127.0.0.1:3306", "root", "12345"));
        con->setSchema("duma");
        score = (int)score;

        std::string str = "INSERT INTO SCORES(scoremass, date, time) VALUES (" + std::to_string(score) + ", CURRENT_DATE(), CURRENT_TIME());";

        std::unique_ptr<sql::PreparedStatement> pstmt(con->prepareStatement(str));
        pstmt->executeUpdate();

        std::cout << "Score saved to database: " << score << std::endl;
    }
    catch (sql::SQLException& e) {
        std::cerr << "Error saving score to database: " << e.what() << std::endl;
    }
}