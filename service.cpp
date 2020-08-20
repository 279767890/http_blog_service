#define MYSQLPP_MYSQL_HEADERS_BURIED
#include "httplib.h"
#include "rapidjson/document.h"
#include <mysql++/mysql++.h>
#include <iostream>
#include <string>
using namespace std;

namespace {
  const string blog_db = "blog_system";
  const string blog_table = "blog_table";
  const string blog_server = "cdb-ad5lpz7y.cd.tencentcdb.com";
  const string blog_user = "root";
  const string blog_pass = "cjl1234@";
  const int blog_port = 10163;
}


int main() {
  using namespace httplib;
  using namespace mysqlpp;
  using namespace rapidjson;

  mysqlpp::Connection conn(false);
  conn.set_option(new mysqlpp::SetCharsetNameOption("utf8"));
  if (conn.connect(blog_db.c_str(), blog_server.c_str(), blog_user.c_str(),
                      blog_pass.c_str(), blog_port)) {
    cout << "success" << endl;
  } else {
    exit(0);
  }

  Server svr;
  svr.Get("/", [](const Request& req, Response& res) {
    cout << "/" << endl;
    res.set_content("hello world!", "text/plain");
  });

  svr.Get("/hi", [](const Request& req, Response& res) {
    cout << "hi" << endl;
    res.set_content("Hi", "text/plain");
  });
  svr.Post("/hello", [](const Request& req, Response& res) {
    res.set_content("hello post", "text/plain");
  });

  svr.Post("/blog", [&](const Request& req, Response& res) {
    Document d;
    if (d.Parse(req.body.c_str()).HasParseError()) {
      return 1;
    }

    auto title = d["title"].GetString();
    auto content = d["content"].GetString();
    auto tag_id = d["tag_id"].GetInt();
    auto create_time = d["create_time"].GetString();
    unique_ptr<char> sql(new char[strlen(content) * 2 + 4096]);
    sprintf(sql.get(), "insert into blog_table values(null, '%s', '%s', '%d', '%s')",
            title, content, tag_id, create_time);
    mysqlpp::Query query = conn.query(string(sql.get()));
    try {
      query.execute();
    } catch(const mysqlpp::BadQuery& er) {
      return -1;
    }
  });

  svr.listen("localhost", 1234);
}