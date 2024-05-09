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
    inventory integer DEFAULT 1,                     -- 库存
    retail_price numeric(6, 2) NOT NULL,             -- 零售价
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

INSERT INTO books (ISBN, title, author, publisher, price, inventory, retail_price) VALUES
('978-0-06-112008-4', 'To Kill a Mockingbird', 'Harper Lee', 'J.B. Lippincott & Co.', 18.99, 12, 24.99),
('978-0-7432-7356-5', 'The Great Gatsby', 'F. Scott Fitzgerald', 'Scribner', 14.99, 15, 19.99),
('978-0-452-28423-4', '1984', 'George Orwell', 'Penguin Books', 16.99, 8, 21.99),
('978-0-06-085052-4', 'Brave New World', 'Aldous Huxley', 'Harper Perennial', 15.99, 10, 20.99),
('978-0-14-017739-8', 'Of Mice and Men', 'John Steinbeck', 'Penguin Books', 13.99, 18, 17.99),
('978-0-14-303943-3', 'The Catcher in the Rye', 'J.D. Salinger', 'Little, Brown and Company', 17.99, 9, 22.99),
('978-0-452-28425-8', 'Animal Farm', 'George Orwell', 'Penguin Books', 15.99, 20, 20.99),
('978-0-06-112241-5', 'The Road', 'Cormac McCarthy', 'Vintage', 17.99, 14, 21.99),
('978-0-7432-7357-2', 'Moby-Dick', 'Herman Melville', 'Harper & Brothers', 19.99, 5, 25.99),
('978-0-14-028333-4', 'Pride and Prejudice', 'Jane Austen', 'Modern Library', 12.99, 22, 16.99),
('978-0-14-028329-7', 'Wuthering Heights', 'Emily Brontë', 'Penguin Books', 14.99, 18, 19.99),
('978-0-7432-7358-9', 'Jane Eyre', 'Charlotte Brontë', 'Penguin Books', 16.99, 7, 21.99),
('978-0-14-144236-4', 'Great Expectations', 'Charles Dickens', 'Penguin Classics', 18.99, 12, 24.99),
('978-0-679-73232-2', 'Crime and Punishment', 'Fyodor Dostoevsky', 'Vintage', 17.99, 15, 22.99),
('978-0-06-093546-7', 'East of Eden', 'John Steinbeck', 'Penguin Books', 19.99, 13, 25.99),
('978-0-14-243720-9', 'Les Misérables', 'Victor Hugo', 'Modern Library', 22.99, 11, 28.99),
('978-0-451-52990-4', 'Anna Karenina', 'Leo Tolstoy', 'Modern Library', 20.99, 16, 26.99),
('978-0-679-74558-1', 'War and Peace', 'Leo Tolstoy', 'Penguin Classics', 24.99, 20, 30.99),
('978-0-374-53315-0', 'One Hundred Years of Solitude', 'Gabriel Garcia Marquez', 'Harper & Row', 21.99, 9, 27.99),
('978-0-679-72232-3', 'The Brothers Karamazov', 'Fyodor Dostoevsky', 'Vintage', 23.99, 12, 29.99),
('978-0-385-74264-4', 'A Tale of Two Cities', 'Charles Dickens', 'Penguin Classics', 18.99, 10, 24.99),
('978-0-14-018639-0', 'Don Quixote', 'Miguel de Cervantes', 'Penguin Classics', 20.99, 17, 26.99),
('978-0-393-32502-3', 'Madame Bovary', 'Gustave Flaubert', 'Penguin Books', 19.99, 8, 24.99),
('978-0-14-303999-0', 'Lolita', 'Vladimir Nabokov', 'Vintage', 17.99, 21, 22.99),
('978-0-06-083867-4', 'Invisible Man', 'Ralph Ellison', 'Vintage', 19.99, 7, 24.99),
('978-0-14-310627-2', 'Beloved', 'Toni Morrison', 'Vintage', 18.99, 19, 23.99),
('978-0-679-73232-3', 'Catch-22', 'Joseph Heller', 'Simon & Schuster', 21.99, 13, 27.99),
('978-0-14-118993-0', 'Ulysses', 'James Joyce', 'Penguin Classics', 23.99, 11, 28.99),
('978-0-393-32500-9', 'Slaughterhouse-Five', 'Kurt Vonnegut', 'Modern Library', 18.99, 16, 24.99),
('978-0-14-017739-9', 'Gone with the Wind', 'Margaret Mitchell', 'Macmillan', 24.99, 15, 30.99);

