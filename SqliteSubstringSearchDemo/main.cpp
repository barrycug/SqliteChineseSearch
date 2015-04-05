//
//  main.c
//  SqliteSubstringSearchDemo
//
//  Created by Hai Feng Kao on 4/6/13.
//  Copyright (c) 2013 com.haifeng. All rights reserved.
//
//  This program demos how to use sqlite FTS (full text search) and character tokenizer to query substrings efficiently

#include <stdio.h>
#include <iostream>

#include <string>

#include<fstream>
#include <sqlite3.h>
#include "character_tokenizer.h"

#include <stdio.h>
#include "csv.h"

using namespace std;
static string removesql = "DROP TABLE jiudian_v;";

/*
static char *createsql = "CREATE VIRTUAL TABLE Book USING fts3(name TEXT NOT NULL, author TEXT, tokenize=character);";
 */
static string createsql = "CREATE VIRTUAL TABLE jiudian_v USING fts3(name TEXT, cnname TEXT,lat DOUBLE, lon DOUBLE, address TEXT,star TEXT,tokenize=character);";

// The query sql commands. Note that they are phrase queries.
/*
static string querysql_prefix = "SELECT * FROM Book WHERE Book MATCH '\"普吉岛\"'";
static string querysql_suffix = "SELECT * FROM Book WHERE Book MATCH '\"度假村\"'";
static string querysql_infix = "SELECT * FROM Book WHERE Book MATCH '\"end\"'";
 */

/*
 ** Register a tokenizer implementation with FTS3 or FTS4.
 */
static int registerTokenizer(
                             sqlite3 *db,
                             char *zName,
                             const sqlite3_tokenizer_module *p
                             ){
    int rc;
    sqlite3_stmt *pStmt;
    const char *zSql = "SELECT fts3_tokenizer(?, ?)";
    
    rc = sqlite3_prepare_v2(db, zSql, -1, &pStmt, 0);
    if( rc!=SQLITE_OK ){
        return rc;
    }
    
    sqlite3_bind_text(pStmt, 1, zName, -1, SQLITE_STATIC);
    sqlite3_bind_blob(pStmt, 2, &p, sizeof(p), SQLITE_STATIC);
    sqlite3_step(pStmt);
    
    return sqlite3_finalize(pStmt);
}
void importdata()
{
    sqlite3 *db;
    char *errMsg = NULL;
    char **result;
    const sqlite3_tokenizer_module *ptr;
    
    char token_name[] = "character";
    string path = "/Users/bzhang/Desktop/puket.sqlite";
    if (sqlite3_open_v2(path.c_str(), &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL)) {
        return ;
    }
    get_character_tokenizer_module(&ptr);
    registerTokenizer(db, token_name, ptr);
    sqlite3_exec(db, removesql.c_str(), 0, 0, &errMsg);
    sqlite3_exec(db, createsql.c_str(), 0, 0, &errMsg);
    
    
    io::CSVReader<11> in("/Users/bzhang/Desktop/jiudiandata2_copy.txt");
    in.read_header(io::ignore_extra_column, "name","cnname","lat","lon","price","address","score","dpscore","dpcount","star","stardesc");
    std::string name;  std::string cnname;double lat; double lon;string price;std::string address;
    std::string score;std::string dpscore;std::string dpcount;std::string star;std::string stardesc;
    while(in.read_row(name,cnname,lat,lon,price,address,score,dpscore,dpcount,star,stardesc)){
        string sql ="INSERT INTO jiudian_v VALUES('"+name+"','"+cnname+"',"+to_string(lat)+","+to_string(lon)+",'"+address+"','"+star+"');";
        size_t nPos= 0;
        nPos = sql.find("\"", nPos);
        while(nPos != string::npos)
        {
            sql.replace(nPos, 1, "");
            nPos = sql.find("\"", nPos);
        }
        sqlite3_exec(db, sql.c_str(), 0, 0, &errMsg);
    }
    sqlite3_close(db);
}
void test()
{
    sqlite3 *db;
    char *errMsg = NULL;
    char **result;
    int i, j;
    int rows, cols;
    const sqlite3_tokenizer_module *ptr;
    
    char token_name[] = "character";
    string path = "/Users/bzhang/Desktop/puket.sqlite";
    if (sqlite3_open_v2(path.c_str(), &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL)) {
        return ;
    }
    get_character_tokenizer_module(&ptr);
    registerTokenizer(db, token_name, ptr);
    string querysql_prefix = "SELECT * FROM jiudian_v WHERE jiudian_v MATCH '\"普吉\"'";
    sqlite3_get_table(db , querysql_prefix.c_str(), &result , &rows, &cols, &errMsg);
    
    printf("query results for %s\n", querysql_prefix.c_str());
    for (i=0;i<=rows;i++) {
        for (j=0;j< cols;j++) {
            printf("%s\t", result[i*cols+j]);
        }
        printf("\n");
    }

}
int main(void)
{
  // importdata();
    test();
}
/*
int main(void)
{
    int i, j;
    int rows, cols;
    sqlite3 *db;
    char *errMsg = NULL;
    char **result;
    const sqlite3_tokenizer_module *ptr;
    char token_name[] = "character";
    
    // create the database
    if (sqlite3_open_v2("example.db", &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL)) {
        return 0;
    }
    
    // get the tokenizer
    get_character_tokenizer_module(&ptr);
    
    // register character tokenizer, note that you need to register it everytime the database is opened
    registerTokenizer(db, token_name, ptr);
    
    // remove existing table
    sqlite3_exec(db, removesql, 0, 0, &errMsg);
    
    // create table with sqlite FTS3 support
    sqlite3_exec(db, createsql, 0, 0, &errMsg);
    
    // insert test data
    sqlite3_exec(db, "INSERT INTO Book VALUES('普吉岛艾琳塔度假村, v1', 'Stern Gamboge');", 0, 0, &errMsg);
    sqlite3_exec(db, "INSERT INTO Book VALUES('普吉岛玛丽莎套房度假村, v2', 'Stern Gamboge');", 0, 0, &errMsg);
    sqlite3_exec(db, "INSERT INTO Book VALUES('普吉岛芭东巴尔米拉度假村, v3', 'Stern Gamboge');", 0, 0, &errMsg);
    sqlite3_exec(db, "INSERT INTO Book VALUES('普吉岛瑞享邦道海滩度假村', 'Alexandre Simon');", 0, 0, &errMsg);
    sqlite3_exec(db, "INSERT INTO Book VALUES('普吉岛科莫雅姆度假村', 'Matera Chapel');", 0, 0, &errMsg);
    sqlite3_exec(db, "INSERT INTO Book VALUES('普吉岛星晨别墅酒店, v3', 'Marobar Sul');", 0, 0, &errMsg);
    
    // test prefix matches
    sqlite3_get_table(db , querysql_prefix, &result , &rows, &cols, &errMsg);
    
    printf("query results for %s\n", querysql_prefix);
    for (i=0;i<=rows;i++) {
        for (j=0;j< cols;j++) {
            printf("%s\t", result[i*cols+j]);
        }
        printf("\n");
    }
    sqlite3_free_table(result);
    printf("\n");
    
    // test suffix matches
    sqlite3_get_table(db , querysql_suffix, &result , &rows, &cols, &errMsg);
    
    printf("query results for %s\n", querysql_suffix);
    for (i=0;i<=rows;i++) {
        for (j=0;j< cols;j++) {
            printf("%s\t", result[i*cols+j]);
        }
        printf("\n");
    }
    sqlite3_free_table(result);
    printf("\n");
    
    // test infix matches
    sqlite3_get_table(db , querysql_infix, &result , &rows, &cols, &errMsg);
    
    printf("query results for %s\n", querysql_infix);
    for (i=0;i<=rows;i++) {
        for (j=0;j< cols;j++) {
            printf("%s\t", result[i*cols+j]);
        }
        printf("\n");
    }
    sqlite3_free_table(result);

    // close database
    sqlite3_close(db);
    
    return 0;
}
 */

