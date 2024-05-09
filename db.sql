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

CREATE TABLE books (
    ISBN character varying(255) PRIMARY KEY,         -- ISBN
    title character varying(255) NOT NULL,           -- 书名
    author character varying(255) NOT NULL,          -- 作者
    publisher character varying(255) NOT NULL,       -- 出版社
    price numeric(6, 2) NOT NULL,                    -- 价格
    inventory integer DEFAULT 1                      -- 库存
);

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

CREATE TABLE bill (
    id serial PRIMARY KEY,                           -- id
    user_id integer NOT NULL,                        -- 用户id
    bill_time timestamp NOT NULL,                    -- 账单时间
    bill_amount numeric(6, 2) NOT NULL,              -- 账单金额
    bill_status integer NOT NULL,                    -- 账单状态 0:支出 1:收入
    -- foreign key
    FOREIGN KEY (user_id) REFERENCES auth_user(id)
);

-- insert superuser while creating the table
INSERT INTO auth_user (username, password, is_superuser, first_name, last_name, email, staff_number, sex, age)
VALUES ('ffa500', 'VwIK2qsXCGMJsNhX$fqiPYlimk1CyExpMX3WISZZ19iCbQlktoerWJIMqNLA=', true, 'jy', 's', 'ffa500@gmail.com', 205, 'M', 22);

INSERT INTO auth_user (username, password, is_superuser, first_name, last_name, email, staff_number, sex, age)
VALUES ('ljq', 'VwIK2qsXCGMJsNhX$fqiPYlimk1CyExpMX3WISZZ19iCbQlktoerWJIMqNLA=', true, 'jq', 'l', 'leng@gmail.com', 106, 'M', 22);

INSERT INTO books (ISBN, title, author, publisher, price, inventory) VALUES 
('978-0-12-958837-5', 'Much Congress especially.', 'Xavier Knox', 'Yang, Wood and Perry', 95.77, 82),
('978-0-12-205846-2', 'Imagine per history some.', 'Jeremy King', 'Frank LLC', 65.29, 24),
('978-1-165-36536-4', 'White.', 'Kelly Robbins', 'Lang, Schmitt and Perez', 36.47, 58),
('978-0-412-40900-4', 'Born plant.', 'Tracey Perez', 'Hill-Hall', 86.59, 26),
('978-1-280-20043-4', 'Chance explain rest sport century president.', 'Victoria Chase', 'Ward, Nguyen and Barton', 55.5, 15),
('978-1-77923-206-9', 'Including explain learn.', 'Michael Davis', 'Wu, Anderson and Anderson', 73.75, 17),
('978-0-598-94901-1', 'Already assume buy event difficult less.', 'Daniel Lewis', 'Rodriguez Group', 64.76, 79),
('978-1-65621-414-0', 'For near executive record.', 'Kenneth Nelson', 'Howard, Chapman and Fisher', 82.75, 49),
('978-0-02-204827-3', 'Change officer young.', 'Jessica Guerra', 'Martinez PLC', 12.74, 67),
('978-0-414-89384-9', 'Wife song name finally.', 'Michael Hernandez', 'Mcguire-Davis', 71.24, 70);

INSERT INTO books (ISBN, title, author, publisher, price, inventory) VALUES 
('978-0-457-78570-5', 'Order it.', 'Molly Lopez', 'Lowe LLC', 34.1, 13),
('978-1-58403-234-2', 'Occur character weight.', 'Alyssa Phillips', 'Briggs, Miller and Gaines', 20.84, 53),
('978-1-253-68360-8', 'Wonder.', 'Dwayne Johnson', 'Ross, Gonzalez and Johnson', 69.62, 46),
('978-0-339-28906-2', 'People plant so modern.', 'Randy Noble', 'Johnson, Figueroa and Martinez', 44.11, 82),
('978-0-406-01384-2', 'We team.', 'Jessica Espinoza', 'Bowers and Sons', 16.86, 60),
('978-0-281-20695-7', 'Popular.', 'Thomas Turner', 'Johnson, Reeves and Shaw', 28.49, 3),
('978-1-58258-048-7', 'Even man friend read arm.', 'Robin Smith', 'Mendez and Sons', 42.89, 63),
('978-1-01-048857-6', 'Current beat bar compare.', 'Olivia Ayala', 'Petersen Group', 36.09, 20),
('978-0-656-84905-5', 'Style would.', 'John Wright', 'Duncan, Martinez and Barrett', 81.66, 95),
('978-1-57683-435-0', 'Part of human.', 'Nancy Hall', 'Jones-Ortega', 26.32, 37);
