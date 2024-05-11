# 数据库 pj1 文档

## 1. 需求分析

本实验设计并实现了一个简单的书城数据库管理系统，具备数据库的创建、删除、查询、更新、插入等功能。在前端界面上，系统提供了直观易用的数据库管理功能，使用户可以通过简单操作方便地管理书籍及相关信息。系统的主要功能模块包括用户管理、书籍管理、购买、进货、捐赠、删除、收支计算以及评论和点赞。

## 2. 概要设计

**用户管理**

用户管理模块包括注册、登录、登出等基本功能，记录用户的基本信息，如用户名、密码、邮箱、性别等。管理员通过后端的 `SQL` 语句创建，而用户则可以通过前端的注册功能进行注册，注册后默认为非管理员用户。根据权限的不同，用户能够执行不同的操作，例如非管理员用户仅能查询和购买书籍，而不能进行进货或删除等操作。管理员用户拥有完整的操作权限。

**书籍管理**

书籍管理模块记录书籍的基本信息，包括 `ISBN` 号、书名、作者、出版社、进货价、零售价、库存等。`ISBN` 号唯一标识每本书籍，可用于查询书籍信息。库存会实时更新，购买操作减少库存，进货操作增加库存。进货价是书籍的进价，零售价是书籍的销售价格。用户可通过书名等信息进行书籍查询。

**进货**

进货模块记录进货时间、数量、书籍 `ISBN` 号等信息，进货操作直接影响库存和收支。系统不仅支持新书进货，还支持对已有书籍进行补货。

**购买**

购买模块允许用户在商城界面购买书籍，并记录购买时间、数量、书籍 `ISBN` 号等信息。购买操作直接影响库存和收支。系统会确保购买的书籍存在且库存充足，结合进货和购买记录以及书籍信息计算收支。

**点赞**

点赞模块允许用户对书籍进行点赞，点赞数量影响书籍的热度。点赞信息实时更新，并显示在书籍信息中，确保用户随时可以在前端界面点赞。

**评论**

评论模块允许用户对书籍发表评论，记录评论时间、内容、用户等信息。所有评论集中存储，可通过书名等信息查询。

**删除**

管理员可删除订单，但删除后订单将从列表中移除。通常不建议删除订单，因为这会影响收支计算且操作不可逆。

**图书馆**

每个图书馆需要维护库存和图书馆名称，并记录每次捐赠所带来的社会声望增长。库存需实时更新。

**捐赠**

捐赠功能仅管理员可用，记录捐赠时间、数量、用户、目标图书馆等信息。每本书的右侧设有捐赠按钮，管理员可点击进行捐赠。系统会检查捐赠书籍是否存在且库存充足。捐赠信息用于计算社会声望，鼓励用户积极参与。

**ER 模型图**

![8](ER.png)

更详细的图，请见`ER.drawio`文件。

**关系模型**

好的，这是每个表的示例关系模型表示：

1. **auth_user**  
   `auth_user(id(PK), username, password, is_superuser, first_name, last_name, email, staff_number, sex, age, is_active)`

2. **books**  
   `books(ISBN(PK), title, author, publisher, price, inventory, retail_price)`

3. **orders**  
   `orders(id(PK), user_id(FK), ISBN(FK), order_time, order_status, order_quantity)`

4. **sale**  
   `sale(id(PK), ISBN(FK), sale_time, sale_quantity, user_id(FK))`

5. **likes**  
   `likes(id(PK), user_id(FK), ISBN(FK), like_time)`

6. **comments**  
   `comments(id(PK), user_id(FK), ISBN(FK), comment_time, comment_content)`

7. **libraries**  
   `libraries(id(PK), total_collection, social_prestige_per_book, library_name)`

8. **donation**  
   `donation(id(PK), user_id(FK), num, donation_time, library_id(FK))`

## 详细设计

**_用户管理_**

这里我简单扩展了一下`auth_user`的信息，具体的表结构如下：

```sql
CREATE TABLE auth_user (
    id serial PRIMARY KEY,                           -- id
    username character varying(255) NOT NULL UNIQUE, -- 用户名
    password character varying(255) NOT NULL,        -- 密码
    is_superuser boolean NOT NULL,                   -- 权限
    first_name character varying(255) NOT NULL,      -- 真实姓名（名）
    last_name character varying(255) NOT NULL,       -- 真实姓名（姓）
    email character varying(255) NOT NULL,           -- 邮箱
    staff_number character varying(255) NOT NULL,    -- 工号
    sex character varying(255) NOT NULL,             -- 性别
    age integer NOT NULL,                            -- 年龄
    is_active boolean NOT NULL DEFAULT true          -- 是否激活
);
```

同时，我创建了`add user`的按钮来对用户进行注册，不管是已经登录过的用户还是未登录的用户，都可以通过这个按钮来注册新用户。但是，默认全部注册为非管理员用户。在后端，我通过一个简单的`insert`语句来实现用户的注册，并且按照注册的顺序，自动分配一个`id`。对密码进行了加密处理，并且储存在数据库中。

具体效果如下：

![1](images/front_user.png)

**书籍管理**

对于书籍的管理，我创建了一个`books`表，具体的表结构如下：

```sql
CREATE TABLE books (
    ISBN character varying(255) PRIMARY KEY,         -- ISBN
    title character varying(255) NOT NULL,           -- 书名
    author character varying(255) NOT NULL,          -- 作者
    publisher character varying(255) NOT NULL,       -- 出版社
    price numeric(6, 2) NOT NULL,                    -- 价格
    inventory integer DEFAULT 1,                     -- 库存
    retail_price numeric(6, 2) NOT NULL,             -- 零售价
);
```

非管理员用户与管理员用户公用这张表，不过操作的前端界面不同，可执行的操作也不同。对于非管理员用户，只能进行查询和购买操作，需要在`bookstore`页面进行操作。具体效果如下，显示了每本书的书名作者出版社价格库存零售价等信息。

![2](images/front_bookstore.png)

如果你是超级管理员，那么你可以去另外一个界面，显示信息是类似的，但是多了一些操作，比如进货，捐赠，添加书籍等。具体效果如下：

![4](images/front_books.png)

用户可以通过书名来进行书籍查询，查询结果会显示在下方的表格中。显示效果与不进行查询时相同，在后端实现中，使用了模糊匹配的方式来进行查询。

```c++
bserv::db_result db_res = tx.exec(
    "select * from books where title like ? order by ISBN ASC limit 10 offset ? ;",
    "%" + search_str + "%",
    (page_id - 1) * 10);
```

**售卖记录**

可以看到，我们的商城上每本书下方都有一个小购物车，点击后会弹出一个购买的对话框，用户可以选择购买的数量，点击购买后会在`sales`表中插入一条记录。具体的表结构如下：

```sql
CREATE TABLE sale (
    id serial PRIMARY KEY,                           -- id
    ISBN character varying(255) NOT NULL,            -- ISBN
    sale_time timestamp NOT NULL,                    -- 销售时间
    sale_quantity integer NOT NULL,                  -- 销售数量
    user_id integer NOT NULL,                        -- 用户id
    -- foreign key
    FOREIGN KEY (user_id) REFERENCES auth_user(id),
    FOREIGN KEY (ISBN) REFERENCES book(ISBN)
);
```

在后端实现中，我通过一个简单的`insert`语句来实现售卖记录的插入，并且按照插入的顺序，自动分配一个`id`。同时，我在`books`表中对库存进行了更新，减去了售卖的数量。具体实现如下：

```c++
// update inventory
bserv::db_result r = tx.exec(
    "update ?"
    "set inventory=inventory-?"
    "where ISBN=? ",
    bserv::db_name("books"),
    get_or_empty(params, "quantity"),
    get_or_empty(params, "ISBN"));

// insert into sale
bserv::db_result r2 = tx.exec(
    "insert into ?"
    "(user_id, ISBN, sale_time, sale_quantity) values"
    "(?, ?, ?, ?)",
    bserv::db_name("sale"),
    get_or_empty(params, "userid"),
    get_or_empty(params, "ISBN"),
    cur_time,
    get_or_empty(params, "quantity"));
```

在另外一个界面中，只有管理员用户可以看到，显示了所有的售卖记录，包括了售卖的时间，售卖的数量，售卖的用户等信息。具体效果如下：

![3](images/front_sale.png.png)

**进货记录**

进货只有管理员用户可以进行，同时有两种方式，一种是进货新书，一种是对已有书籍进行补货。具体的表结构如下：

```sql
CREATE TABLE orders (
    id serial PRIMARY KEY,                           -- id
    user_id integer NOT NULL,                        -- 用户id
    ISBN character varying(255) NOT NULL,            -- ISBN
    order_time timestamp NOT NULL,                   -- 下单时间
    order_status integer NOT NULL,                   -- 订单状态 0:未付款 1:已付款 2:已到货 3:已取消
    order_quantity integer NOT NULL,                 -- 订单数量
    -- foreign key
    FOREIGN KEY (user_id) REFERENCES auth_user(id),
    FOREIGN KEY (ISBN) REFERENCES book(ISBN)
);
```

如果是进货新书，我们需要点击最上端的`Add book`按钮，然后填写书籍的信息，包括书名，作者，出版社，价格，进货数量，进货价，零售价等信息。同时，在后端实现中，我通过一个简单的`insert`语句来实现进货记录的插入，并且按照插入的顺序，自动分配一个`id`。并且在`books`表中对库存进行了更新，增加了进货的数量。类似地，对于补货，我们需要点击每本书的右侧的`restock`按钮，只需要填写进货的数量即可，操作更为便捷。我们以补货为例：

```c++
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
```

这样还有一个好处，就是我们可以通过`orders`和`sale`表来计算收支，具体的实现的`SQL`语句相对复杂一些:

```c++
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
```

与进货相对应的也有删除进货记录的功能，只有管理员用户可以进行，点击每条进货记录的右侧的`delete`按钮，即可删除进货记录。具体实现如下：

```c++
bserv::db_result r = tx.exec(
    "delete from ?"
    "where ISBN=? ",
    bserv::db_name("books"),
    get_or_empty(params, "ISBN"));
```

当点击`Check income and expenditure`按钮时，会显示当前的收支情况，如果是正数，会恭喜你，你赚钱了，如果是负数，那么你需要注意了，可能是进货太多了，导致了亏损。

![5](images/front_check_income.png)

**点赞**

点赞功能是用户对书籍进行点赞，点赞数量影响书籍的热度。点赞信息实时更新，并显示在书籍信息中，确保用户随时可以在前端界面点赞。具体的表结构如下：

```sql
CREATE TABLE sale (
    id serial PRIMARY KEY,                           -- id
    ISBN character varying(255) NOT NULL,            -- ISBN
    sale_time timestamp NOT NULL,                    -- 销售时间
    sale_quantity integer NOT NULL,                  -- 销售数量
    user_id integer NOT NULL,                        -- 用户id
    -- foreign key
    FOREIGN KEY (user_id) REFERENCES auth_user(id),
    FOREIGN KEY (ISBN) REFERENCES book(ISBN)
);
```

在后端实现中，因为点赞信息是要暴露给所有用户的，所以不能使用`ISBN`作为索引，因此，需要连接`books`表和`likes`表，然后一起打包返回给前端。具体实现如下：

```c++
db_res = tx.exec("SELECT b.ISBN AS ISBN, b.title AS title, b.author, b.publisher, b.retail_price, b.inventory, COUNT(l.id) AS like_count "
                    "FROM books b "
                    "LEFT JOIN likes l ON b.ISBN = l.ISBN "
                    "GROUP BY b.ISBN, b.title "
                    "ORDER BY b.ISBN ASC "
                    "LIMIT 9 OFFSET ?;",
                    (page_id - 1) * 9);
```

在前端实现中，我们可以看到每本书的右侧有一个小心心，点击后会进行点赞，同时会显示点赞的数量。

**评论**

评论功能是用户对书籍发表评论，记录评论时间、内容、用户等信息。所有评论集中存储，可通过书名等信息查询。具体的表结构如下：

```sql
CREATE TABLE comments (
    id serial PRIMARY KEY,                           -- id
    user_id integer NOT NULL,                        -- 用户id
    ISBN character varying(255) NOT NULL,            -- ISBN
    comment_time timestamp NOT NULL,                 -- 评论时间
    comment_content text NOT NULL,                   -- 评论内容
    -- foreign key
    FOREIGN KEY (user_id) REFERENCES auth_user(id),
    FOREIGN KEY (ISBN) REFERENCES books(ISBN)
);
```

前端页面单独设置了一个`comments`页面，用于显示所有的评论，包括评论的书籍，评论的内容等，也能够通过书名来进行查询。因为实现与之前的查询类似，这里就不再赘述。

![6](images/front_comment.png)

**捐赠**

用户可以选择将书籍捐赠给图书馆，捐赠功能仅管理员可用，记录捐赠时间、数量、用户、目标图书馆等信息。每本书的右侧设有捐赠按钮，管理员可点击进行捐赠。系统会检查捐赠书籍是否存在且库存充足。具体的表结构如下：

```sql
CREATE TABLE libraries (
    id serial PRIMARY KEY,                           -- id
    total_collection integer NOT NULL,               -- 总藏书量
    social_prestige_per_book numeric(6, 2) NOT NULL, -- 每捐一本书获得的社会声誉
    library_name character varying(255) NOT NULL     -- 图书馆名称
);

CREATE TABLE donation (
    id serial PRIMARY KEY,                           -- id
    user_id integer NOT NULL,                        -- 用户id
    num integer NOT NULL,                            -- 捐书数量
    donation_time timestamp NOT NULL,                -- 捐书时间
    library_id integer NOT NULL,                     -- 图书馆id
    -- foreign key
    FOREIGN KEY (user_id) REFERENCES auth_user(id),
    FOREIGN KEY (library_id) REFERENCES libraries(id)
);
```

在前端实现中，我们可以看到每本书的右侧有一个手托着爱心的图片，点击后可以根据所有图书馆的名称来选择捐赠的对象，然后填写捐赠的数量，点击`Donate`按钮即可完成捐赠。具体实现如下：

```c++
bserv::db_result r = tx.exec(
    "update ?"
    "set inventory=inventory-?"
    "where ISBN=? ",
    bserv::db_name("books"),
    get_or_empty(params, "quantity"),
    get_or_empty(params, "ISBN"));

std::string cur_time = get_current_time_as_string();

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
```

每次捐赠我们需要同时做三件事，一是在`donation`表中插入一条记录，二是在`libraries`表中更新总藏书量，三是在`books`表中更新库存。这样我们就可以通过`donation`表来计算社会声誉，具体的实现的`SQL`语句相对复杂一些:

```c++
bserv::db_result db_res = tx.exec("SELECT SUM(d.num * l.social_prestige_per_book) AS total_social_prestige "
                                    "FROM donation d "
                                    "JOIN libraries l ON d.library_id = l.id;");
```

当点击图书馆页面的`Check social prestige`按钮时，会显示当前的社会声誉，社会声誉只能是一个非负数。

![7](images/front_lib.png)

## 部署方法

## 部署方法

**依赖安装**

请参阅 `bserv` 目录下的 [README.md] 文件获取详细的依赖安装指南。

**数据库导入**

请打开 `db.sql` 文件，将其中的 `SQL` 语句复制到 `PostgreSQL` 中`bserv`数据库中执行，以创建数据库和表。

**编译**

确保 `template` 目录下包含 9 个 `HTML` 文件，并在 `WebApp` 目录中确保存在 `handler.cpp`、`handler.h` 和 `WebApp.cpp` 文件。然后使用 `Visual Studio` 进行编译。

**运行**

在前端界面中，输入 `http://localhost:8080/` 以访问主界面，并根据需求执行相应操作。