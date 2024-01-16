#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include "sqlite3.h"

#include "nlohmann/json.hpp"
static int callback(void *NotUsed, int argc, char **argv, char **azColName){
    int i;
    for(i=0; i<argc; i++){
        printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    }
    printf("\n\n");
    return 0;
}

void insertUserData(const char *database, const char *name, const char *surname, const char *email, const char *password, const char *mobile) {
    sqlite3 *db;
    char *zErrMsg = 0;
    int rc;

    // Try to open the database. If it does not exist, SQLite will create a new one.
    rc = sqlite3_open(database, &db);
    if (rc) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));

        // Create the database if it does not exist
        rc = sqlite3_open_v2(database, &db, SQLITE_OPEN_CREATE | SQLITE_OPEN_READWRITE, nullptr);
        if (rc != SQLITE_OK) {
            fprintf(stderr, "Can't create database: %s\n", sqlite3_errmsg(db));
            sqlite3_close(db);
            return;
        }
    }

    // Insert data into the User table
    char insertDataSQL[512];
    snprintf(insertDataSQL, sizeof(insertDataSQL),
             "INSERT INTO User (name, surname, email, password, mobile) VALUES ('%s', '%s', '%s', '%s', '%s');",
             name, surname, email, password, mobile);

    rc = sqlite3_exec(db, insertDataSQL, callback, 0, &zErrMsg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    } else {
        printf("Data inserted/updated successfully.\n");
    }

    // Close the database
    sqlite3_close(db);
}


void retrieveUserData(const char *database) {
    sqlite3 *db;
    char *zErrMsg = 0;
    int rc;

    // Try to open the database.
    rc = sqlite3_open(database, &db);
    if (rc) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }

    // Execute a SELECT statement to retrieve user data
    const char *selectDataSQL = "SELECT * FROM User;";
    rc = sqlite3_exec(db, selectDataSQL, callback, 0, &zErrMsg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }

    // Close the database
    sqlite3_close(db);
}

int main(int argc, char *argv[])
{
    sqlite3 *db;
    char *zErrMsg = 0;
    int rc;

    // Try to open the database. If it does not exist, SQLite will create a new one.
    rc = sqlite3_open("adesh.db", &db);
    if( rc == SQLITE_OK){
        qDebug()<<"Database open/create success";
    }
    // Create the User table if it does not exist
    const char *createTableSQL = "CREATE TABLE IF NOT EXISTS User ("
                                 "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                                 "name TEXT,"
                                 "surname TEXT,"
                                 "email TEXT,"
                                 "password TEXT,"
                                 "mobile TEXT"
                                 ");";

    rc = sqlite3_exec(db, createTableSQL, callback, 0, &zErrMsg);
    if( rc != SQLITE_OK ){
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    } else {
        qDebug("User table created or already exists.\n");
    }

    //insertUserData("adesh.db","Adesh","Singh","adeshworkmail@gmail.com","asdfghjk","7772076022");



    printf("\nRetrieving user data:\n");
    retrieveUserData("adesh.db");

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
    QGuiApplication app(argc, argv);


    nlohmann::json jsn;

    QQmlApplicationEngine engine;
    const QUrl url(QStringLiteral("qrc:/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
        &app, [url](QObject *obj, const QUrl &objUrl) {
            if (!obj && url == objUrl)
                QCoreApplication::exit(-1);
        }, Qt::QueuedConnection);
    engine.load(url);

    return app.exec();
}
