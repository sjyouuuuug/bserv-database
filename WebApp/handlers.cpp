#include "handlers.h"

#include <vector>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <unordered_map>

#include "rendering.h"

std::string get_current_time_as_string()
{
	std::time_t now = std::time(nullptr);
	std::tm ltm;
	localtime_s(&ltm, &now);

	std::stringstream ss;
	ss << std::put_time(&ltm, "%Y-%m-%d %H:%M:%S");

	return ss.str();
}

// register an orm mapping (to convert the db query results into
// json objects).
// the db query results contain several rows, each has a number of
// fields. the order of `make_db_field<Type[i]>(name[i])` in the
// initializer list corresponds to these fields (`Type[0]` and
// `name[0]` correspond to field[0], `Type[1]` and `name[1]`
// correspond to field[1], ...). `Type[i]` is the type you want
// to convert the field value to, and `name[i]` is the identifier
// with which you want to store the field in the json object, so
// if the returned json object is `obj`, `obj[name[i]]` will have
// the type of `Type[i]` and store the value of field[i].
bserv::db_relation_to_object orm_user{
	bserv::make_db_field<int>("id"),
	bserv::make_db_field<std::string>("username"),
	bserv::make_db_field<std::string>("password"),
	bserv::make_db_field<bool>("is_superuser"),
	bserv::make_db_field<std::string>("first_name"),
	bserv::make_db_field<std::string>("last_name"),
	bserv::make_db_field<std::string>("email"),
	bserv::make_db_field<std::string>("staff_number"),
	bserv::make_db_field<std::string>("sex"),
	bserv::make_db_field<int>("age"),
	bserv::make_db_field<bool>("is_activate")};

bserv::db_relation_to_object orm_books{
	bserv::make_db_field<std::string>("ISBN"),
	bserv::make_db_field<std::string>("title"),
	bserv::make_db_field<std::string>("author"),
	bserv::make_db_field<std::string>("publisher"),
	bserv::make_db_field<float>("price"),
	bserv::make_db_field<int>("inventory")};

bserv::db_relation_to_object orm_orders{
	bserv::make_db_field<int>("id"),
	bserv::make_db_field<std::string>("user_id"),
	bserv::make_db_field<std::string>("ISBN"),
	bserv::make_db_field<std::string>("order_time"),
	bserv::make_db_field<int>("order_status"), // 0: unpaid, 1: paid, 2: shipped, 3: received
	bserv::make_db_field<int>("order_quantity")};

bserv::db_relation_to_object orm_sale{
	bserv::make_db_field<int>("id"),
	bserv::make_db_field<std::string>("ISBN"),
	bserv::make_db_field<std::string>("sale_time"),
	bserv::make_db_field<int>("sale_quantity"),
	bserv::make_db_field<std::string>("user_id")};

bserv::db_relation_to_object orm_likes{
	bserv::make_db_field<int>("id"),
	bserv::make_db_field<std::string>("ISBN"),
	bserv::make_db_field<std::string>("user_id"),
	bserv::make_db_field<std::string>("like_time")};

bserv::db_relation_to_object orm_likes_books{
	bserv::make_db_field<std::string>("ISBN"),
	bserv::make_db_field<std::string>("title"),
	bserv::make_db_field<std::string>("author"),
	bserv::make_db_field<std::string>("publisher"),
	bserv::make_db_field<float>("retail_price"),
	bserv::make_db_field<int>("inventory"),
	bserv::make_db_field<int>("like_count")};

bserv::db_relation_to_object orm_comments{
	bserv::make_db_field<int>("id"),
	bserv::make_db_field<std::string>("user_id"),
	bserv::make_db_field<std::string>("ISBN"),
	bserv::make_db_field<std::string>("comment_time"),
	bserv::make_db_field<std::string>("comment_content")};

bserv::db_relation_to_object orm_comments_books{
	bserv::make_db_field<std::string>("ISBN"),
	bserv::make_db_field<std::string>("title"),
	bserv::make_db_field<std::string>("comment_text")};

bserv::db_relation_to_object orm_libraries{
	bserv::make_db_field<int>("id"),
	bserv::make_db_field<std::string>("total_collection"),
	bserv::make_db_field<float>("social_prestige_per_book"),
	bserv::make_db_field<std::string>("library_name")};

std::optional<boost::json::object> get_user(
	bserv::db_transaction &tx,
	const boost::json::string &username)
{
	bserv::db_result r = tx.exec(
		"select * from auth_user where username = ?", username);
	lginfo << r.query(); // this is how you log info
	return orm_user.convert_to_optional(r);
}

std::optional<boost::json::object> get_book(
	bserv::db_transaction &tx,
	const boost::json::string &ISBN)
{
	bserv::db_result r = tx.exec(
		"select * from books where ISBN = ?", ISBN);
	lginfo << r.query(); // this is how you log info
	return orm_books.convert_to_optional(r);
}

std::string get_or_empty(
	boost::json::object &obj,
	const std::string &key)
{
	return obj.count(key) ? obj[key].as_string().c_str() : "";
}

// if you want to manually modify the response,
// the return type should be `std::nullopt_t`,
// and the return value should be `std::nullopt`.
std::nullopt_t hello(
	bserv::response_type &response,
	std::shared_ptr<bserv::session_type> session_ptr)
{
	bserv::session_type &session = *session_ptr;
	boost::json::object obj;
	if (session.count("user"))
	{
		// NOTE: modifications to sessions must be performed
		// BEFORE referencing objects in them. this is because
		// modifications might invalidate referenced objects.
		// in this example, "count" might be added to `session`,
		// which should be performed first.
		// then `user` can be referenced safely.
		if (!session.count("count"))
		{
			session["count"] = 0;
		}
		auto &user = session["user"].as_object();
		session["count"] = session["count"].as_int64() + 1;
		obj = {
			{"welcome", user["username"]},
			{"count", session["count"]}};
	}
	else
	{
		obj = {{"msg", "hello, world!"}};
	}
	// the response body is a string,
	// so the `obj` should be serialized
	response.body() = boost::json::serialize(obj);
	response.prepare_payload(); // this line is important!
	return std::nullopt;
}

// if you return a json object, the serialization
// is performed automatically.
boost::json::object user_register(
	bserv::request_type &request,
	// the json object is obtained from the request body,
	// as well as the url parameters
	boost::json::object &&params,
	std::shared_ptr<bserv::db_connection> conn)
{
	if (request.method() != boost::beast::http::verb::post)
	{
		throw bserv::url_not_found_exception{};
	}
	if (params.count("username") == 0)
	{
		return {
			{"success", false},
			{"message", "`username` is required"}};
	}
	if (params.count("password") == 0)
	{
		return {
			{"success", false},
			{"message", "`password` is required"}};
	}
	auto username = params["username"].as_string();
	bserv::db_transaction tx{conn};
	auto opt_user = get_user(tx, username);
	if (opt_user.has_value())
	{
		return {
			{"success", false},
			{"message", "`username` existed"}};
	}
	auto password = params["password"].as_string();
	bserv::db_result r = tx.exec(
		"insert into ? "
		"(?, password, is_superuser, "
		"first_name, last_name, email, is_active, staff_number, sex, age) values "
		"(?, ?, ?, ?, ?, ?, ?, ?, ?, ?)",
		bserv::db_name("auth_user"),
		bserv::db_name("username"),
		username,
		bserv::utils::security::encode_password(
			password.c_str()),
		false,
		get_or_empty(params, "first_name"),
		get_or_empty(params, "last_name"),
		get_or_empty(params, "email"), true,
		get_or_empty(params, "staff_number"),
		get_or_empty(params, "sex"), get_or_empty(params, "age"));
	lginfo << r.query();
	tx.commit(); // you must manually commit changes
	return {
		{"success", true},
		{"message", "user registered"}};
}

boost::json::object book_register(
	bserv::request_type &request,
	// the json object is obtained from the request body,
	// as well as the url parameters
	boost::json::object &&params,
	std::shared_ptr<bserv::db_connection> conn)
{
	if (request.method() != boost::beast::http::verb::post)
	{
		throw bserv::url_not_found_exception{};
	}
	if (params.count("ISBN") == 0)
	{
		return {
			{"success", false},
			{"message", "`ISBN` is required"}};
	}
	if (params.count("title") == 0)
	{
		return {
			{"success", false},
			{"message", "`title` is required"}};
	}
	if (params.count("author") == 0)
	{
		return {
			{"success", false},
			{"message", "`author` is required"}};
	}
	if (params.count("publisher") == 0)
	{
		return {
			{"success", false},
			{"message", "`publisher` is required"}};
	}
	if (params.count("price") == 0)
	{
		return {
			{"success", false},
			{"message", "`price` is required"}};
	}
	if (params.count("retail_price") == 0)
	{
		return {
			{"success", false},
			{"message", "`retail_price` is required"}};
	}
	if (params.count("quantity") == 0)
	{
		return {
			{"success", false},
			{"message", "`quantity` is required"}};
	}
	auto ISBN = params["ISBN"].as_string();
	bserv::db_transaction tx{conn};
	auto opt_book = get_book(tx, ISBN);
	if (opt_book.has_value())
	{
		return {
			{"success", false},
			{"message", "`book` existed"}};
	}

	auto ISBN_str = boost::json::value_to<std::string>(params.at("ISBN"));

	// check ifISBN start with "978" or "979"
	if (ISBN_str.substr(0, 3) != "978" && ISBN_str.substr(0, 3) != "979")
	{
		return {
			{"success", false},
			{"message", "`ISBN` must start with 978 or 979"}};
	}

	bserv::db_result r = tx.exec(
		"insert into ? "
		"(ISBN, title, author, publisher, price, inventory, retail_price) values "
		"(?, ?, ?, ?, ?, ?, ?)",
		bserv::db_name("books"),
		get_or_empty(params, "ISBN"),
		get_or_empty(params, "title"),
		get_or_empty(params, "author"),
		get_or_empty(params, "publisher"),
		get_or_empty(params, "price"),
		get_or_empty(params, "quantity"),
		get_or_empty(params, "retail_price"));

	std::string cur_time = get_current_time_as_string();
	bserv::db_result r2 = tx.exec(
		"insert into ?"
		"(user_id, ISBN, order_time, order_status, order_quantity) values"
		"(?, ?, ?, ?, ?)",
		bserv::db_name("orders"),
		get_or_empty(params, "userid"),
		get_or_empty(params, "ISBN"),
		cur_time,
		2,
		get_or_empty(params, "quantity"));
	lginfo << r.query();
	lginfo << r2.query();
	tx.commit(); // you must manually commit changes
	return {
		{"success", true},
		{"message", "books added"}};
}

boost::json::object compute_total_income_and_expenditure(
	bserv::request_type &request,
	// the json object is obtained from the request body,
	// as well as the url parameters
	boost::json::object &&params,
	std::shared_ptr<bserv::db_connection> conn)
{
	if (request.method() != boost::beast::http::verb::post)
	{
		throw bserv::url_not_found_exception{};
	}

	bserv::db_transaction tx{conn};

	bserv::db_result db_res = tx.exec("SELECT "
									  "(ts.sales_total - tp.purchase_total) AS total_income_and_expenditure "
									  "FROM "
									  "( "
									  "    SELECT SUM(o.order_quantity * b.price) AS purchase_total "
									  "    FROM orders o "
									  "    JOIN books b ON o.ISBN = b.ISBN "
									  "    WHERE o.order_status = 2 "
									  ") AS tp, "
									  "( "
									  "    SELECT SUM(s.sale_quantity * b.retail_price) AS sales_total "
									  "    FROM sale s "
									  "    JOIN books b ON s.ISBN = b.ISBN "
									  ") AS ts;");

	double total = (*db_res.begin())[0].as<double>();
	std::string message;
	if (total < 0)
	{
		message = "Oops! You have to work hard to make profit!\n";
		message += "Total income and expenditure: " + std::to_string(total);
	}
	else
	{
		message = "Congratulations! You have made profit!\n";
		message += "Total income and expenditure: " + std::to_string(total);
	}

	return {
		{"success", true},
		{"message", message}};
}

boost::json::object compute_social_prestige(
	bserv::request_type &request,
	// the json object is obtained from the request body,
	// as well as the url parameters
	boost::json::object &&params,
	std::shared_ptr<bserv::db_connection> conn)
{
	if (request.method() != boost::beast::http::verb::post)
	{
		throw bserv::url_not_found_exception{};
	}

	bserv::db_transaction tx{conn};

	bserv::db_result db_res = tx.exec("SELECT SUM(d.num * l.social_prestige_per_book) AS total_social_prestige "
									  "FROM donation d "
									  "JOIN libraries l ON d.library_id = l.id;");

	double total = (*db_res.begin())[0].as<double>();
	std::string message;
	message = "You have gained " + std::to_string(total) + " social prestige points!";

	return {
		{"success", true},
		{"message", message}};
}

boost::json::object user_login(
	bserv::request_type &request,
	boost::json::object &&params,
	std::shared_ptr<bserv::db_connection> conn,
	std::shared_ptr<bserv::session_type> session_ptr)
{
	if (request.method() != boost::beast::http::verb::post)
	{
		throw bserv::url_not_found_exception{};
	}
	if (params.count("username") == 0)
	{
		return {
			{"success", false},
			{"message", "`username` is required"}};
	}
	if (params.count("password") == 0)
	{
		return {
			{"success", false},
			{"message", "`password` is required"}};
	}
	auto username = params["username"].as_string();
	bserv::db_transaction tx{conn};
	auto opt_user = get_user(tx, username);
	if (!opt_user.has_value())
	{
		return {
			{"success", false},
			{"message", "invalid username/password"}};
	}
	auto &user = opt_user.value();
	auto password = params["password"].as_string();
	auto encoded_password = user["password"].as_string();
	if (!bserv::utils::security::check_password(
			password.c_str(), encoded_password.c_str()))
	{
		return {
			{"success", false},
			{"message", "invalid username/password"}};
	}
	bserv::session_type &session = *session_ptr;
	session["user"] = user;
	return {
		{"success", true},
		{"message", "login successfully"}};
}

boost::json::object find_user(
	std::shared_ptr<bserv::db_connection> conn,
	const std::string &username)
{
	bserv::db_transaction tx{conn};
	auto user = get_user(tx, username.c_str());
	if (!user.has_value())
	{
		return {
			{"success", false},
			{"message", "requested user does not exist"}};
	}
	user.value().erase("id");
	user.value().erase("password");
	return {
		{"success", true},
		{"user", user.value()}};
}

boost::json::object user_logout(
	std::shared_ptr<bserv::session_type> session_ptr)
{
	bserv::session_type &session = *session_ptr;
	if (session.count("user"))
	{
		session.erase("user");
	}
	return {
		{"success", true},
		{"message", "logout successfully"}};
}

boost::json::object send_request(
	std::shared_ptr<bserv::session_type> session,
	std::shared_ptr<bserv::http_client> client_ptr,
	boost::json::object &&params)
{
	// post for response:
	// auto res = client_ptr->post(
	//     "localhost", "8080", "/echo", {{"msg", "request"}}
	// );
	// return {{"response", boost::json::parse(res.body())}};
	// -------------------------------------------------------
	// - if it takes longer than 30 seconds (by default) to
	// - get the response, this will raise a read timeout
	// -------------------------------------------------------
	// post for json response (json value, rather than json
	// object, is returned):
	auto obj = client_ptr->post_for_value(
		"localhost", "8080", "/echo", {{"request", params}});
	if (session->count("cnt") == 0)
	{
		(*session)["cnt"] = 0;
	}
	(*session)["cnt"] = (*session)["cnt"].as_int64() + 1;
	return {{"response", obj}, {"cnt", (*session)["cnt"]}};
}

boost::json::object echo(
	boost::json::object &&params)
{
	return {{"echo", params}};
}

// websocket
std::nullopt_t ws_echo(
	std::shared_ptr<bserv::session_type> session,
	std::shared_ptr<bserv::websocket_server> ws_server)
{
	ws_server->write_json((*session)["cnt"]);
	while (true)
	{
		try
		{
			std::string data = ws_server->read();
			ws_server->write(data);
		}
		catch (bserv::websocket_closed &)
		{
			break;
		}
	}
	return std::nullopt;
}

std::nullopt_t serve_static_files(
	bserv::response_type &response,
	const std::string &path)
{
	return serve(response, path);
}

std::nullopt_t index(
	const std::string &template_path,
	std::shared_ptr<bserv::session_type> session_ptr,
	bserv::response_type &response,
	boost::json::object &context)
{
	bserv::session_type &session = *session_ptr;
	if (session.contains("user"))
	{
		context["user"] = session["user"];
	}
	return render(response, template_path, context);
}

std::nullopt_t index_page(
	std::shared_ptr<bserv::session_type> session_ptr,
	bserv::response_type &response)
{
	boost::json::object context;
	return index("index.html", session_ptr, response, context);
}

std::nullopt_t form_login(
	bserv::request_type &request,
	bserv::response_type &response,
	boost::json::object &&params,
	std::shared_ptr<bserv::db_connection> conn,
	std::shared_ptr<bserv::session_type> session_ptr)
{
	lgdebug << params << std::endl;
	auto context = user_login(request, std::move(params), conn, session_ptr);
	lginfo << "login: " << context << std::endl;
	return index("index.html", session_ptr, response, context);
}

std::nullopt_t form_logout(
	std::shared_ptr<bserv::session_type> session_ptr,
	bserv::response_type &response)
{
	auto context = user_logout(session_ptr);
	lginfo << "logout: " << context << std::endl;
	return index("index.html", session_ptr, response, context);
}

std::nullopt_t redirect_to_users(
	std::shared_ptr<bserv::db_connection> conn,
	std::shared_ptr<bserv::session_type> session_ptr,
	bserv::response_type &response,
	int page_id,
	boost::json::object &&context)
{
	lgdebug << "view users: " << page_id << std::endl;
	bserv::db_transaction tx{conn};
	bserv::db_result db_res = tx.exec("select count(*) from auth_user;");
	lginfo << db_res.query();
	std::size_t total_users = (*db_res.begin())[0].as<std::size_t>();
	lgdebug << "total users: " << total_users << std::endl;
	int total_pages = (int)total_users / 10;
	if (total_users % 10 != 0)
		++total_pages;
	lgdebug << "total pages: " << total_pages << std::endl;
	db_res = tx.exec("select * from auth_user limit 10 offset ?;", (page_id - 1) * 10);
	lginfo << db_res.query();
	auto users = orm_user.convert_to_vector(db_res);
	boost::json::array json_users;
	for (auto &user : users)
	{
		json_users.push_back(user);
	}
	boost::json::object pagination;
	if (total_pages != 0)
	{
		pagination["total"] = total_pages;
		if (page_id > 1)
		{
			pagination["previous"] = page_id - 1;
		}
		if (page_id < total_pages)
		{
			pagination["next"] = page_id + 1;
		}
		int lower = page_id - 3;
		int upper = page_id + 3;
		if (page_id - 3 > 2)
		{
			pagination["left_ellipsis"] = true;
		}
		else
		{
			lower = 1;
		}
		if (page_id + 3 < total_pages - 1)
		{
			pagination["right_ellipsis"] = true;
		}
		else
		{
			upper = total_pages;
		}
		pagination["current"] = page_id;
		boost::json::array pages_left;
		for (int i = lower; i < page_id; ++i)
		{
			pages_left.push_back(i);
		}
		pagination["pages_left"] = pages_left;
		boost::json::array pages_right;
		for (int i = page_id + 1; i <= upper; ++i)
		{
			pages_right.push_back(i);
		}
		pagination["pages_right"] = pages_right;
		context["pagination"] = pagination;
	}
	context["users"] = json_users;
	return index("users.html", session_ptr, response, context);
}

std::nullopt_t redirect_to_orders(
	std::shared_ptr<bserv::db_connection> conn,
	std::shared_ptr<bserv::session_type> session_ptr,
	bserv::response_type &response,
	int page_id,
	boost::json::object &&context)
{
	lgdebug << "view orders: " << page_id << std::endl;
	bserv::db_transaction tx{conn};
	bserv::db_result db_res = tx.exec("select count(*) from orders;");
	lginfo << db_res.query();
	std::size_t total_users = (*db_res.begin())[0].as<std::size_t>();
	lgdebug << "total orders: " << total_users << std::endl;
	int total_pages = (int)total_users / 10;
	if (total_users % 10 != 0)
		++total_pages;
	lgdebug << "total pages: " << total_pages << std::endl;
	db_res = tx.exec("select * from orders order by ISBN ASC limit 10 offset ?;", (page_id - 1) * 10);
	lginfo << db_res.query();
	auto orders = orm_orders.convert_to_vector(db_res);
	boost::json::array json_users;
	for (auto &order : orders)
	{
		json_users.push_back(order);
	}
	boost::json::object pagination;
	if (total_pages != 0)
	{
		pagination["total"] = total_pages;
		if (page_id > 1)
		{
			pagination["previous"] = page_id - 1;
		}
		if (page_id < total_pages)
		{
			pagination["next"] = page_id + 1;
		}
		int lower = page_id - 3;
		int upper = page_id + 3;
		if (page_id - 3 > 2)
		{
			pagination["left_ellipsis"] = true;
		}
		else
		{
			lower = 1;
		}
		if (page_id + 3 < total_pages - 1)
		{
			pagination["right_ellipsis"] = true;
		}
		else
		{
			upper = total_pages;
		}
		pagination["current"] = page_id;
		boost::json::array pages_left;
		for (int i = lower; i < page_id; ++i)
		{
			pages_left.push_back(i);
		}
		pagination["pages_left"] = pages_left;
		boost::json::array pages_right;
		for (int i = page_id + 1; i <= upper; ++i)
		{
			pages_right.push_back(i);
		}
		pagination["pages_right"] = pages_right;
		context["pagination"] = pagination;
	}
	context["orders"] = json_users;
	return index("orders.html", session_ptr, response, context);
}

std::nullopt_t redirect_to_sale(
	std::shared_ptr<bserv::db_connection> conn,
	std::shared_ptr<bserv::session_type> session_ptr,
	bserv::response_type &response,
	int page_id,
	boost::json::object &&context)
{
	lgdebug << "view orders: " << page_id << std::endl;
	bserv::db_transaction tx{conn};
	bserv::db_result db_res = tx.exec("select count(*) from sale;");
	lginfo << db_res.query();
	std::size_t total_users = (*db_res.begin())[0].as<std::size_t>();
	lgdebug << "total sales: " << total_users << std::endl;
	int total_pages = (int)total_users / 10;
	if (total_users % 10 != 0)
		++total_pages;
	lgdebug << "total pages: " << total_pages << std::endl;
	db_res = tx.exec("select * from sale limit 10 offset ?;", (page_id - 1) * 10);
	lginfo << db_res.query();
	auto sales = orm_sale.convert_to_vector(db_res);
	boost::json::array json_users;
	for (auto &sale : sales)
	{
		json_users.push_back(sale);
	}
	boost::json::object pagination;
	if (total_pages != 0)
	{
		pagination["total"] = total_pages;
		if (page_id > 1)
		{
			pagination["previous"] = page_id - 1;
		}
		if (page_id < total_pages)
		{
			pagination["next"] = page_id + 1;
		}
		int lower = page_id - 3;
		int upper = page_id + 3;
		if (page_id - 3 > 2)
		{
			pagination["left_ellipsis"] = true;
		}
		else
		{
			lower = 1;
		}
		if (page_id + 3 < total_pages - 1)
		{
			pagination["right_ellipsis"] = true;
		}
		else
		{
			upper = total_pages;
		}
		pagination["current"] = page_id;
		boost::json::array pages_left;
		for (int i = lower; i < page_id; ++i)
		{
			pages_left.push_back(i);
		}
		pagination["pages_left"] = pages_left;
		boost::json::array pages_right;
		for (int i = page_id + 1; i <= upper; ++i)
		{
			pages_right.push_back(i);
		}
		pagination["pages_right"] = pages_right;
		context["pagination"] = pagination;
	}
	context["sales"] = json_users;
	return index("sale.html", session_ptr, response, context);
}

std::nullopt_t redirect_to_libraries(
	std::shared_ptr<bserv::db_connection> conn,
	std::shared_ptr<bserv::session_type> session_ptr,
	bserv::response_type &response,
	int page_id,
	boost::json::object &&context)
{
	lgdebug << "view orders: " << page_id << std::endl;
	bserv::db_transaction tx{conn};
	bserv::db_result db_res = tx.exec("select count(*) from libraries;");
	lginfo << db_res.query();
	std::size_t total_users = (*db_res.begin())[0].as<std::size_t>();
	lgdebug << "total libraries: " << total_users << std::endl;
	int total_pages = (int)total_users / 10;
	if (total_users % 10 != 0)
		++total_pages;
	lgdebug << "total pages: " << total_pages << std::endl;
	db_res = tx.exec("select * from libraries limit 10 offset ?;", (page_id - 1) * 10);
	lginfo << db_res.query();
	auto libraries = orm_libraries.convert_to_vector(db_res);
	boost::json::array json_users;
	for (auto &library : libraries)
	{
		json_users.push_back(library);
	}
	boost::json::object pagination;
	if (total_pages != 0)
	{
		pagination["total"] = total_pages;
		if (page_id > 1)
		{
			pagination["previous"] = page_id - 1;
		}
		if (page_id < total_pages)
		{
			pagination["next"] = page_id + 1;
		}
		int lower = page_id - 3;
		int upper = page_id + 3;
		if (page_id - 3 > 2)
		{
			pagination["left_ellipsis"] = true;
		}
		else
		{
			lower = 1;
		}
		if (page_id + 3 < total_pages - 1)
		{
			pagination["right_ellipsis"] = true;
		}
		else
		{
			upper = total_pages;
		}
		pagination["current"] = page_id;
		boost::json::array pages_left;
		for (int i = lower; i < page_id; ++i)
		{
			pages_left.push_back(i);
		}
		pagination["pages_left"] = pages_left;
		boost::json::array pages_right;
		for (int i = page_id + 1; i <= upper; ++i)
		{
			pages_right.push_back(i);
		}
		pagination["pages_right"] = pages_right;
		context["pagination"] = pagination;
	}
	context["libraries"] = json_users;
	return index("libraries.html", session_ptr, response, context);
}

std::nullopt_t redirect_to_books(
	std::shared_ptr<bserv::db_connection> conn,
	std::shared_ptr<bserv::session_type> session_ptr,
	bserv::response_type &response,
	int page_id,
	boost::json::object &&context)
{
	lgdebug << "view books: " << page_id << std::endl;
	bserv::db_transaction tx{conn};
	bserv::db_result db_res = tx.exec("select count(*) from books;");
	lginfo << db_res.query();
	std::size_t total_users = (*db_res.begin())[0].as<std::size_t>();
	lgdebug << "total books: " << total_users << std::endl;
	int total_pages = (int)total_users / 10;
	if (total_users % 10 != 0)
		++total_pages;
	lgdebug << "total pages: " << total_pages << std::endl;
	db_res = tx.exec("select * from books order by ISBN ASC limit 10 offset ?;", (page_id - 1) * 10);
	bserv::db_result db_res_1 = tx.exec("select * from libraries;");
	lginfo << db_res.query();
	auto books = orm_books.convert_to_vector(db_res);
	auto libraries = orm_libraries.convert_to_vector(db_res_1);
	boost::json::array json_users, json_libraries;
	for (auto &book : books)
	{
		json_users.push_back(book);
	}
	for (auto &library : libraries)
	{
		json_libraries.push_back(library);
	}
	boost::json::object pagination;
	if (total_pages != 0)
	{
		pagination["total"] = total_pages;
		if (page_id > 1)
		{
			pagination["previous"] = page_id - 1;
		}
		if (page_id < total_pages)
		{
			pagination["next"] = page_id + 1;
		}
		int lower = page_id - 3;
		int upper = page_id + 3;
		if (page_id - 3 > 2)
		{
			pagination["left_ellipsis"] = true;
		}
		else
		{
			lower = 1;
		}
		if (page_id + 3 < total_pages - 1)
		{
			pagination["right_ellipsis"] = true;
		}
		else
		{
			upper = total_pages;
		}
		pagination["current"] = page_id;
		boost::json::array pages_left;
		for (int i = lower; i < page_id; ++i)
		{
			pages_left.push_back(i);
		}
		pagination["pages_left"] = pages_left;
		boost::json::array pages_right;
		for (int i = page_id + 1; i <= upper; ++i)
		{
			pages_right.push_back(i);
		}
		pagination["pages_right"] = pages_right;
		context["pagination"] = pagination;
	}
	context["books"] = json_users;
	context["libraries"] = json_libraries;
	return index("books.html", session_ptr, response, context);
}

std::nullopt_t redirect_to_comments(
	std::shared_ptr<bserv::db_connection> conn,
	std::shared_ptr<bserv::session_type> session_ptr,
	bserv::response_type &response,
	int page_id,
	boost::json::object &&context)
{
	lgdebug << "view books: " << page_id << std::endl;
	bserv::db_transaction tx{conn};
	bserv::db_result db_res = tx.exec("select count(*) from books;");
	lginfo << db_res.query();
	std::size_t total_users = (*db_res.begin())[0].as<std::size_t>();
	lgdebug << "total books: " << total_users << std::endl;
	int total_pages = (int)total_users / 10;
	if (total_users % 10 != 0)
		++total_pages;
	lgdebug << "total pages: " << total_pages << std::endl;
	db_res = tx.exec("SELECT b.ISBN, b.title, COALESCE(string_agg(c.comment_content, '; '), 'No comments') AS comment_text "
					 "FROM books b "
					 "LEFT JOIN comments c ON b.ISBN = c.ISBN "
					 "GROUP BY b.ISBN, b.title "
					 "ORDER BY b.ISBN ASC "
					 "limit 10 offset ?;",
					 (page_id - 1) * 10);
	lginfo << db_res.query();
	auto comments = orm_comments_books.convert_to_vector(db_res);
	boost::json::array json_users;
	for (auto &comment : comments)
	{
		json_users.push_back(comment);
	}
	boost::json::object pagination;
	if (total_pages != 0)
	{
		pagination["total"] = total_pages;
		if (page_id > 1)
		{
			pagination["previous"] = page_id - 1;
		}
		if (page_id < total_pages)
		{
			pagination["next"] = page_id + 1;
		}
		int lower = page_id - 3;
		int upper = page_id + 3;
		if (page_id - 3 > 2)
		{
			pagination["left_ellipsis"] = true;
		}
		else
		{
			lower = 1;
		}
		if (page_id + 3 < total_pages - 1)
		{
			pagination["right_ellipsis"] = true;
		}
		else
		{
			upper = total_pages;
		}
		pagination["current"] = page_id;
		boost::json::array pages_left;
		for (int i = lower; i < page_id; ++i)
		{
			pages_left.push_back(i);
		}
		pagination["pages_left"] = pages_left;
		boost::json::array pages_right;
		for (int i = page_id + 1; i <= upper; ++i)
		{
			pages_right.push_back(i);
		}
		pagination["pages_right"] = pages_right;
		context["pagination"] = pagination;
	}
	context["comments"] = json_users;
	return index("comments.html", session_ptr, response, context);
}

std::nullopt_t redirect_to_bookstore(
	std::shared_ptr<bserv::db_connection> conn,
	std::shared_ptr<bserv::session_type> session_ptr,
	bserv::response_type &response,
	int page_id,
	boost::json::object &&context)
{
	lgdebug << "view books: " << page_id << std::endl;
	bserv::db_transaction tx{conn};
	bserv::db_result db_res = tx.exec("select count(*) from books;");
	lginfo << db_res.query();
	std::size_t total_users = (*db_res.begin())[0].as<std::size_t>();
	lgdebug << "total books: " << total_users << std::endl;
	int total_pages = (int)total_users / 10;
	if (total_users % 10 != 0)
		++total_pages;
	lgdebug << "total pages: " << total_pages << std::endl;
	db_res = tx.exec("SELECT b.ISBN AS ISBN, b.title AS title, b.author, b.publisher, b.retail_price, b.inventory, COUNT(l.id) AS like_count "
					 "FROM books b "
					 "LEFT JOIN likes l ON b.ISBN = l.ISBN "
					 "GROUP BY b.ISBN, b.title "
					 "ORDER BY b.ISBN ASC "
					 "LIMIT 10 OFFSET ?;",
					 (page_id - 1) * 10);

	lginfo << db_res.query();
	auto books = orm_likes_books.convert_to_vector(db_res);
	boost::json::array json_users;
	for (auto &book : books)
	{
		json_users.push_back(book);
	}
	boost::json::object pagination;
	if (total_pages != 0)
	{
		pagination["total"] = total_pages;
		if (page_id > 1)
		{
			pagination["previous"] = page_id - 1;
		}
		if (page_id < total_pages)
		{
			pagination["next"] = page_id + 1;
		}
		int lower = page_id - 3;
		int upper = page_id + 3;
		if (page_id - 3 > 2)
		{
			pagination["left_ellipsis"] = true;
		}
		else
		{
			lower = 1;
		}
		if (page_id + 3 < total_pages - 1)
		{
			pagination["right_ellipsis"] = true;
		}
		else
		{
			upper = total_pages;
		}
		pagination["current"] = page_id;
		boost::json::array pages_left;
		for (int i = lower; i < page_id; ++i)
		{
			pages_left.push_back(i);
		}
		pagination["pages_left"] = pages_left;
		boost::json::array pages_right;
		for (int i = page_id + 1; i <= upper; ++i)
		{
			pages_right.push_back(i);
		}
		pagination["pages_right"] = pages_right;
		context["pagination"] = pagination;
	}
	context["books"] = json_users;
	return index("bookstore.html", session_ptr, response, context);
}

std::nullopt_t view_users(
	std::shared_ptr<bserv::db_connection> conn,
	std::shared_ptr<bserv::session_type> session_ptr,
	bserv::response_type &response,
	const std::string &page_num)
{
	int page_id = std::stoi(page_num);
	boost::json::object context;
	return redirect_to_users(conn, session_ptr, response, page_id, std::move(context));
}

std::nullopt_t view_orders(
	std::shared_ptr<bserv::db_connection> conn,
	std::shared_ptr<bserv::session_type> session_ptr,
	bserv::response_type &response,
	const std::string &page_num)
{
	int page_id = std::stoi(page_num);
	boost::json::object context;
	return redirect_to_orders(conn, session_ptr, response, page_id, std::move(context));
}

std::nullopt_t view_sale(
	std::shared_ptr<bserv::db_connection> conn,
	std::shared_ptr<bserv::session_type> session_ptr,
	bserv::response_type &response,
	const std::string &page_num)
{
	int page_id = std::stoi(page_num);
	boost::json::object context;
	return redirect_to_sale(conn, session_ptr, response, page_id, std::move(context));
}

std::nullopt_t view_libraries(
	std::shared_ptr<bserv::db_connection> conn,
	std::shared_ptr<bserv::session_type> session_ptr,
	bserv::response_type &response,
	const std::string &page_num)
{
	int page_id = std::stoi(page_num);
	boost::json::object context;
	return redirect_to_libraries(conn, session_ptr, response, page_id, std::move(context));
}

std::nullopt_t view_books(
	std::shared_ptr<bserv::db_connection> conn,
	std::shared_ptr<bserv::session_type> session_ptr,
	bserv::response_type &response,
	const std::string &page_num)
{
	int page_id = std::stoi(page_num);
	boost::json::object context;
	return redirect_to_books(conn, session_ptr, response, page_id, std::move(context));
}

std::nullopt_t view_comments(
	std::shared_ptr<bserv::db_connection> conn,
	std::shared_ptr<bserv::session_type> session_ptr,
	bserv::response_type &response,
	const std::string &page_num)
{
	int page_id = std::stoi(page_num);
	boost::json::object context;
	return redirect_to_comments(conn, session_ptr, response, page_id, std::move(context));
}

std::nullopt_t view_bookstore(
	std::shared_ptr<bserv::db_connection> conn,
	std::shared_ptr<bserv::session_type> session_ptr,
	bserv::response_type &response,
	const std::string &page_num)
{
	int page_id = std::stoi(page_num);
	boost::json::object context;
	return redirect_to_bookstore(conn, session_ptr, response, page_id, std::move(context));
}

std::nullopt_t form_add_user(
	bserv::request_type &request,
	bserv::response_type &response,
	boost::json::object &&params,
	std::shared_ptr<bserv::db_connection> conn,
	std::shared_ptr<bserv::session_type> session_ptr)
{
	boost::json::object context = user_register(request, std::move(params), conn);
	return redirect_to_users(conn, session_ptr, response, 1, std::move(context));
}

std::nullopt_t form_add_book(
	bserv::request_type &request,
	bserv::response_type &response,
	boost::json::object &&params,
	std::shared_ptr<bserv::db_connection> conn,
	std::shared_ptr<bserv::session_type> session_ptr)
{
	boost::json::object context = book_register(request, std::move(params), conn);
	return redirect_to_books(conn, session_ptr, response, 1, std::move(context));
}

std::nullopt_t get_total_income_and_expenditure(
	bserv::request_type &request,
	bserv::response_type &response,
	boost::json::object &&params,
	std::shared_ptr<bserv::db_connection> conn,
	std::shared_ptr<bserv::session_type> session_ptr)
{
	boost::json::object context = compute_total_income_and_expenditure(request, std::move(params), conn);
	return redirect_to_sale(conn, session_ptr, response, 1, std::move(context));
}

std::nullopt_t get_social_prestige(
	bserv::request_type &request,
	bserv::response_type &response,
	boost::json::object &&params,
	std::shared_ptr<bserv::db_connection> conn,
	std::shared_ptr<bserv::session_type> session_ptr)
{
	boost::json::object context = compute_social_prestige(request, std::move(params), conn);
	return redirect_to_libraries(conn, session_ptr, response, 1, std::move(context));
}

// TODO: implement order_register function
std::nullopt_t form_add_order(
	bserv::request_type &request,
	bserv::response_type &response,
	boost::json::object &&params,
	std::shared_ptr<bserv::db_connection> conn,
	std::shared_ptr<bserv::session_type> session_ptr)
{
	boost::json::object context = book_register(request, std::move(params), conn);
	return redirect_to_orders(conn, session_ptr, response, 1, std::move(context));
}

boost::json::object book_delete(
	bserv::request_type &request,
	boost::json::object &&params,
	std::shared_ptr<bserv::db_connection> conn)
{
	if (request.method() != boost::beast::http::verb::post)
	{
		throw bserv::url_not_found_exception{};
	}

	if (params.count("ISBN") == 0)
	{
		return {
			{"success", false},
			{"message", "`ISBN` is required"}};
	}

	auto ISBN = params["ISBN"].as_string();
	bserv::db_transaction tx{conn};

	bserv::db_result r = tx.exec(
		"delete from ?"
		"where ISBN=? ",
		bserv::db_name("books"),
		get_or_empty(params, "ISBN"));

	lginfo << r.query();
	tx.commit(); // you must manually commit changes
	return {
		{"success", true},
		{"message", "book deleted"}};
}

boost::json::object like_hit(
	bserv::request_type &request,
	boost::json::object &&params,
	std::shared_ptr<bserv::db_connection> conn)
{
	if (request.method() != boost::beast::http::verb::post)
	{
		throw bserv::url_not_found_exception{};
	}
	if (params.count("ISBN") == 0)
	{
		return {
			{"success", false},
			{"message", "`ISBN` is required"}};
	}
	if (params.count("userid") == 0)
	{
		return {
			{"success", false},
			{"message", "`userid` is required"}};
	}

	auto ISBN = params["ISBN"].as_string();
	std::string cur_time = get_current_time_as_string();
	bserv::db_transaction tx{conn};

	bserv::db_result r = tx.exec(
		"insert into ?"
		"(user_id, ISBN, like_time) values"
		"(?, ?, ?)",
		bserv::db_name("likes"),
		get_or_empty(params, "userid"),
		get_or_empty(params, "ISBN"),
		cur_time);

	lginfo << r.query();
	tx.commit(); // you must manually commit changes
	return {
		{"success", true},
		{"message", "like hit"}};
}

boost::json::object book_purchase(
	bserv::request_type &request,
	boost::json::object &&params,
	std::shared_ptr<bserv::db_connection> conn)
{
	if (request.method() != boost::beast::http::verb::post)
	{
		throw bserv::url_not_found_exception{};
	}

	if (params.count("ISBN") == 0)
	{
		return {
			{"success", false},
			{"message", "`ISBN` is required"}};
	}
	if (params.count("quantity") == 0)
	{
		return {
			{"success", false},
			{"message", "`quantity` is required"}};
	}
	if (params.count("userid") == 0)
	{
		return {
			{"success", false},
			{"message", "`userid` is required"}};
	}

	auto ISBN = params["ISBN"].as_string();
	bserv::db_transaction tx{conn};

	bserv::db_result r = tx.exec(
		"update ?"
		"set inventory=inventory-?"
		"where ISBN=? ",
		bserv::db_name("books"),
		get_or_empty(params, "quantity"),
		get_or_empty(params, "ISBN"));

	std::string cur_time = get_current_time_as_string();

	bserv::db_result r2 = tx.exec(
		"insert into ?"
		"(user_id, ISBN, sale_time, sale_quantity) values"
		"(?, ?, ?, ?)",
		bserv::db_name("sale"),
		get_or_empty(params, "userid"),
		get_or_empty(params, "ISBN"),
		cur_time,
		get_or_empty(params, "quantity"));

	lginfo << r.query();
	// lginfo << r2.query();
	tx.commit(); // you must manually commit changes
	return {
		{"success", true},
		{"message", "book purchased"}};
}

boost::json::object book_restock(
	bserv::request_type &request,
	boost::json::object &&params,
	std::shared_ptr<bserv::db_connection> conn)
{
	if (request.method() != boost::beast::http::verb::post)
	{
		throw bserv::url_not_found_exception{};
	}

	if (params.count("ISBN") == 0)
	{
		return {
			{"success", false},
			{"message", "`ISBN` is required"}};
	}
	if (params.count("quantity") == 0)
	{
		return {
			{"success", false},
			{"message", "`quantity` is required"}};
	}
	if (params.count("userid") == 0)
	{
		return {
			{"success", false},
			{"message", "`userid` is required"}};
	}

	auto ISBN = params["ISBN"].as_string();
	bserv::db_transaction tx{conn};

	bserv::db_result r = tx.exec(
		"update ?"
		"set inventory=inventory+?"
		"where ISBN=? ",
		bserv::db_name("books"),
		get_or_empty(params, "quantity"),
		get_or_empty(params, "ISBN"));

	std::string cur_time = get_current_time_as_string();

	bserv::db_result r2 = tx.exec(
		"insert into ?"
		"(user_id, ISBN, order_time, order_status, order_quantity) values"
		"(?, ?, ?, ?, ?)",
		bserv::db_name("orders"),
		get_or_empty(params, "userid"),
		get_or_empty(params, "ISBN"),
		cur_time,
		2,
		get_or_empty(params, "quantity"));

	lginfo << r.query();
	lginfo << r2.query();
	tx.commit(); // you must manually commit changes
	return {
		{"success", true},
		{"message", "book restocked"}};
}

boost::json::object book_donate(
	bserv::request_type &request,
	boost::json::object &&params,
	std::shared_ptr<bserv::db_connection> conn)
{
	if (request.method() != boost::beast::http::verb::post)
	{
		throw bserv::url_not_found_exception{};
	}

	if (params.count("ISBN") == 0)
	{
		return {
			{"success", false},
			{"message", "`ISBN` is required"}};
	}
	if (params.count("quantity") == 0)
	{
		return {
			{"success", false},
			{"message", "`quantity` is required"}};
	}
	if (params.count("userid") == 0)
	{
		return {
			{"success", false},
			{"message", "`userid` is required"}};
	}
	if (params.count("library_name") == 0)
	{
		return {
			{"success", false},
			{"message", "`library_name` is required"}};
	}

	auto ISBN = params["ISBN"].as_string();
	bserv::db_transaction tx{conn};

	bserv::db_result db_res = tx.exec(
		"select id from ?"
		"where library_name = ?",
		bserv::db_name("libraries"),
		get_or_empty(params, "library_name"));

	int library_id = (*db_res.begin())[0].as<int>();

	bserv::db_result r = tx.exec(
		"update ?"
		"set inventory=inventory-?"
		"where ISBN=? ",
		bserv::db_name("books"),
		get_or_empty(params, "quantity"),
		get_or_empty(params, "ISBN"));

	std::string cur_time = get_current_time_as_string();

	lginfo << r.query();
	lginfo << library_id;

	bserv::db_result r2 = tx.exec(
		"insert into ? "
		"(user_id, num, donation_time, library_id) values "
		"(?, ?, ?, ?) ",
		bserv::db_name("donation"),
		get_or_empty(params, "userid"),
		get_or_empty(params, "quantity"),
		cur_time,
		library_id);

	// add to library collection
	bserv::db_result r3 = tx.exec(
		"update ? "
		"set total_collection = total_collection + ? "
		"where id = ? ;",
		bserv::db_name("libraries"),
		get_or_empty(params, "quantity"),
		library_id);

	lginfo << r2.query();
	lginfo << r3.query();
	tx.commit(); // you must manually commit changes
	return {
		{"success", true},
		{"message", "donate successfully"}};
}

boost::json::object comment_add(
	bserv::request_type &request,
	boost::json::object &&params,
	std::shared_ptr<bserv::db_connection> conn)
{
	if (request.method() != boost::beast::http::verb::post)
	{
		throw bserv::url_not_found_exception{};
	}

	if (params.count("ISBN") == 0)
	{
		return {
			{"success", false},
			{"message", "`ISBN` is required"}};
	}
	if (params.count("comment") == 0)
	{
		return {
			{"success", false},
			{"message", "`comment` cannot be empty"}};
	}
	if (params.count("userid") == 0)
	{
		return {
			{"success", false},
			{"message", "`userid` is required"}};
	}

	auto ISBN = params["ISBN"].as_string();
	bserv::db_transaction tx{conn};

	auto bk = get_book(tx, ISBN);
	lginfo << bk.value();
	if (!bk.has_value())
	{
		return {
			{"success", false},
			{"message", "book does not exist"}};
	}

	std::string cur_time = get_current_time_as_string();

	bserv::db_result r2 = tx.exec(
		"insert into ? "
		"(user_id, ISBN, comment_time, comment_content) values "
		"(?, ?, ?, ?) ",
		bserv::db_name("comments"),
		get_or_empty(params, "userid"),
		get_or_empty(params, "ISBN"),
		cur_time,
		get_or_empty(params, "comment"));

	lginfo << r2.query();
	tx.commit(); // you must manually commit changes
	return {
		{"success", true},
		{"message", "comment added"}};
}

std::nullopt_t delete_book(
	bserv::request_type &request,
	bserv::response_type &response,
	boost::json::object &&params,
	std::shared_ptr<bserv::db_connection> conn,
	std::shared_ptr<bserv::session_type> session_ptr)
{
	boost::json::object context = book_delete(request, std::move(params), conn);
	return redirect_to_books(conn, session_ptr, response, 1, std::move(context));
}

std::nullopt_t hit_like(
	bserv::request_type &request,
	bserv::response_type &response,
	boost::json::object &&params,
	std::shared_ptr<bserv::db_connection> conn,
	std::shared_ptr<bserv::session_type> session_ptr)
{
	boost::json::object context = like_hit(request, std::move(params), conn);
	return redirect_to_bookstore(conn, session_ptr, response, 1, std::move(context));
}

std::nullopt_t purchase_book(
	bserv::request_type &request,
	bserv::response_type &response,
	boost::json::object &&params,
	std::shared_ptr<bserv::db_connection> conn,
	std::shared_ptr<bserv::session_type> session_ptr)
{
	boost::json::object context = book_purchase(request, std::move(params), conn);
	return redirect_to_bookstore(conn, session_ptr, response, 1, std::move(context));
}

std::nullopt_t restock_book(
	bserv::request_type &request,
	bserv::response_type &response,
	boost::json::object &&params,
	std::shared_ptr<bserv::db_connection> conn,
	std::shared_ptr<bserv::session_type> session_ptr)
{
	boost::json::object context = book_restock(request, std::move(params), conn);
	return redirect_to_books(conn, session_ptr, response, 1, std::move(context));
}

std::nullopt_t donate_book(
	bserv::request_type &request,
	bserv::response_type &response,
	boost::json::object &&params,
	std::shared_ptr<bserv::db_connection> conn,
	std::shared_ptr<bserv::session_type> session_ptr)
{
	boost::json::object context = book_donate(request, std::move(params), conn);
	return redirect_to_books(conn, session_ptr, response, 1, std::move(context));
}

std::nullopt_t add_new_comments(
	bserv::request_type &request,
	bserv::response_type &response,
	boost::json::object &&params,
	std::shared_ptr<bserv::db_connection> conn,
	std::shared_ptr<bserv::session_type> session_ptr)
{
	boost::json::object context = comment_add(request, std::move(params), conn);
	return redirect_to_comments(conn, session_ptr, response, 1, std::move(context));
}

std::nullopt_t redirect_to_search(
	std::shared_ptr<bserv::db_connection> conn,
	std::shared_ptr<bserv::session_type> session_ptr,
	bserv::response_type &response,
	int page_id,
	boost::json::object &&context,
	boost::json::object &&params)
{
	lgdebug << "view users: " << page_id << std::endl;
	auto search_str = boost::json::value_to<std::string>(params.at("search"));
	bserv::db_transaction tx{conn};
	bserv::db_result db_res = tx.exec("select count(*) from books where title like ?;", "%" + search_str + "%");
	lginfo << db_res.query();
	std::size_t total_users = (*db_res.begin())[0].as<std::size_t>();
	lgdebug << "total users: " << total_users << std::endl;
	int total_pages = (int)total_users / 10;
	if (total_users % 10 != 0)
		++total_pages;
	lgdebug << "total pages: " << total_pages << std::endl;

	db_res = tx.exec(
		"select * from books where title like ? order by ISBN ASC limit 10 offset ? ;",
		"%" + search_str + "%",
		(page_id - 1) * 10);

	bserv::db_result db_res_1 = tx.exec("select * from libraries;");

	lginfo << db_res.query();
	auto lists = orm_books.convert_to_vector(db_res);
	auto libraries = orm_libraries.convert_to_vector(db_res_1);
	boost::json::array json_lists, json_libraies;
	for (auto &list : lists)
	{
		json_lists.push_back(list);
	}
	for (auto &library : libraries)
	{
		json_libraies.push_back(library);
	}
	boost::json::object pagination;
	if (total_pages != 0)
	{
		pagination["total"] = total_pages;
		if (page_id > 1)
		{
			pagination["previous"] = page_id - 1;
		}
		if (page_id < total_pages)
		{
			pagination["next"] = page_id + 1;
		}
		int lower = page_id - 3;
		int upper = page_id + 3;
		if (page_id - 3 > 2)
		{
			pagination["left_ellipsis"] = true;
		}
		else
		{
			lower = 1;
		}
		if (page_id + 3 < total_pages - 1)
		{
			pagination["right_ellipsis"] = true;
		}
		else
		{
			upper = total_pages;
		}
		pagination["current"] = page_id;
		boost::json::array pages_left;
		for (int i = lower; i < page_id; ++i)
		{
			pages_left.push_back(i);
		}
		pagination["pages_left"] = pages_left;
		boost::json::array pages_right;
		for (int i = page_id + 1; i <= upper; ++i)
		{
			pages_right.push_back(i);
		}
		pagination["pages_right"] = pages_right;
		context["pagination"] = pagination;
	}
	context["books"] = json_lists;
	context["libraries"] = json_libraies;
	return index("books.html", session_ptr, response, context);
}

std::nullopt_t redirect_to_search_bookstore(
	std::shared_ptr<bserv::db_connection> conn,
	std::shared_ptr<bserv::session_type> session_ptr,
	bserv::response_type &response,
	int page_id,
	boost::json::object &&context,
	boost::json::object &&params)
{
	lgdebug << "view users: " << page_id << std::endl;
	auto search_str = boost::json::value_to<std::string>(params.at("search"));
	bserv::db_transaction tx{conn};
	bserv::db_result db_res = tx.exec("select count(*) from books where title like ?;", "%" + search_str + "%");
	lginfo << db_res.query();
	std::size_t total_users = (*db_res.begin())[0].as<std::size_t>();
	lgdebug << "total users: " << total_users << std::endl;
	int total_pages = (int)total_users / 10;
	if (total_users % 10 != 0)
		++total_pages;
	lgdebug << "total pages: " << total_pages << std::endl;

	db_res = tx.exec("SELECT b.ISBN AS ISBN, b.title AS title, b.author, b.publisher, b.retail_price, b.inventory, COUNT(l.id) AS like_count "
					 "FROM books b "
					 "LEFT JOIN likes l ON b.ISBN = l.ISBN "
					 "WHERE title LIKE ? "
					 "GROUP BY b.ISBN, b.title "
					 "ORDER BY b.ISBN ASC "
					 "LIMIT 10 OFFSET ?;",
					 "%" + search_str + "%",
					 (page_id - 1) * 10);

	lginfo << db_res.query();
	auto lists = orm_likes_books.convert_to_vector(db_res);
	boost::json::array json_lists;
	for (auto &list : lists)
	{
		json_lists.push_back(list);
	}
	boost::json::object pagination;
	if (total_pages != 0)
	{
		pagination["total"] = total_pages;
		if (page_id > 1)
		{
			pagination["previous"] = page_id - 1;
		}
		if (page_id < total_pages)
		{
			pagination["next"] = page_id + 1;
		}
		int lower = page_id - 3;
		int upper = page_id + 3;
		if (page_id - 3 > 2)
		{
			pagination["left_ellipsis"] = true;
		}
		else
		{
			lower = 1;
		}
		if (page_id + 3 < total_pages - 1)
		{
			pagination["right_ellipsis"] = true;
		}
		else
		{
			upper = total_pages;
		}
		pagination["current"] = page_id;
		boost::json::array pages_left;
		for (int i = lower; i < page_id; ++i)
		{
			pages_left.push_back(i);
		}
		pagination["pages_left"] = pages_left;
		boost::json::array pages_right;
		for (int i = page_id + 1; i <= upper; ++i)
		{
			pages_right.push_back(i);
		}
		pagination["pages_right"] = pages_right;
		context["pagination"] = pagination;
	}
	context["books"] = json_lists;
	return index("bookstore.html", session_ptr, response, context);
}

std::nullopt_t redirect_to_search_comments(
	std::shared_ptr<bserv::db_connection> conn,
	std::shared_ptr<bserv::session_type> session_ptr,
	bserv::response_type &response,
	int page_id,
	boost::json::object &&context,
	boost::json::object &&params)
{
	lgdebug << "view users: " << page_id << std::endl;
	auto search_str = boost::json::value_to<std::string>(params.at("search"));
	bserv::db_transaction tx{conn};
	bserv::db_result db_res = tx.exec("select count(*) from books where title like ?;", "%" + search_str + "%");
	lginfo << db_res.query();
	std::size_t total_users = (*db_res.begin())[0].as<std::size_t>();
	lgdebug << "total users: " << total_users << std::endl;
	int total_pages = (int)total_users / 10;
	if (total_users % 10 != 0)
		++total_pages;
	lgdebug << "total pages: " << total_pages << std::endl;

	db_res = tx.exec("SELECT b.ISBN, b.title, COALESCE(string_agg(c.comment_content, '; '), 'No comments') AS comment_text "
					 "FROM books b "
					 "LEFT JOIN comments c ON b.ISBN = c.ISBN "
					 "WHERE title LIKE ? "
					 "GROUP BY b.ISBN, b.title "
					 "ORDER BY b.ISBN ASC "
					 "limit 10 offset ?;",
					 "%" + search_str + "%",
					 (page_id - 1) * 10);

	lginfo << db_res.query();
	auto lists = orm_comments_books.convert_to_vector(db_res);
	boost::json::array json_lists;
	for (auto &list : lists)
	{
		json_lists.push_back(list);
	}
	boost::json::object pagination;
	if (total_pages != 0)
	{
		pagination["total"] = total_pages;
		if (page_id > 1)
		{
			pagination["previous"] = page_id - 1;
		}
		if (page_id < total_pages)
		{
			pagination["next"] = page_id + 1;
		}
		int lower = page_id - 3;
		int upper = page_id + 3;
		if (page_id - 3 > 2)
		{
			pagination["left_ellipsis"] = true;
		}
		else
		{
			lower = 1;
		}
		if (page_id + 3 < total_pages - 1)
		{
			pagination["right_ellipsis"] = true;
		}
		else
		{
			upper = total_pages;
		}
		pagination["current"] = page_id;
		boost::json::array pages_left;
		for (int i = lower; i < page_id; ++i)
		{
			pages_left.push_back(i);
		}
		pagination["pages_left"] = pages_left;
		boost::json::array pages_right;
		for (int i = page_id + 1; i <= upper; ++i)
		{
			pages_right.push_back(i);
		}
		pagination["pages_right"] = pages_right;
		context["pagination"] = pagination;
	}
	context["comments"] = json_lists;
	return index("comments.html", session_ptr, response, context);
}

std::nullopt_t view_search(
	std::shared_ptr<bserv::db_connection> conn,
	std::shared_ptr<bserv::session_type> session_ptr,
	bserv::response_type &response,
	boost::json::object &&params,
	const std::string &page_num)
{
	int page_id = std::stoi(page_num);
	boost::json::object context;
	return redirect_to_search(conn, session_ptr, response, page_id, std::move(context), std::move(params));
}

std::nullopt_t view_search_bookstore(
	std::shared_ptr<bserv::db_connection> conn,
	std::shared_ptr<bserv::session_type> session_ptr,
	bserv::response_type &response,
	boost::json::object &&params,
	const std::string &page_num)
{
	int page_id = std::stoi(page_num);
	boost::json::object context;
	return redirect_to_search_bookstore(conn, session_ptr, response, page_id, std::move(context), std::move(params));
}

std::nullopt_t view_search_comments(
	std::shared_ptr<bserv::db_connection> conn,
	std::shared_ptr<bserv::session_type> session_ptr,
	bserv::response_type &response,
	boost::json::object &&params,
	const std::string &page_num)
{
	int page_id = std::stoi(page_num);
	boost::json::object context;
	return redirect_to_search_comments(conn, session_ptr, response, page_id, std::move(context), std::move(params));
}