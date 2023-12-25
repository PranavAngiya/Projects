CREATE DATABASE  IF NOT EXISTS Clothes_to_my_Heart;
USE Clothes_to_my_Heart;

DROP TABLE IF EXISTS bids;
DROP TABLE IF EXISTS clothing;
DROP TABLE IF EXISTS account;
DROP TABLE IF EXISTS qna;
DROP TABLE IF EXISTS wishes;

CREATE TABLE account(
    username varchar(50) PRIMARY KEY,
    password varchar(50),
    email varchar(100) UNIQUE,
    acctype ENUM('Admin', 'End_User', 'Customer_Representative') DEFAULT 'End_User',
    earnings float DEFAULT 0,
    alerts boolean DEFAULT 0
);



CREATE TABLE clothing(
    Clothing_ID int NOT NULL AUTO_INCREMENT PRIMARY KEY,
    Seller_Username varchar(50),
    Clothing_Category ENUM('Shirt', 'Pant', 'Belt', 'Shoe') NOT NULL,
    Brand varchar(20),
    Color varchar(15),
    Clothing_Description varchar(100),
    Current_Bid_Value float NOT NULL,
    Closing_Date_Time datetime DEFAULT NULL,
    Reserve_Price varchar(10),
    Min_Bid_Increment DECIMAL(10,2),
    Winner_Username varchar(50),
    Winner_Acknowledged bool default false,
    style varchar(15),
    size varchar(5),
    gender ENUM('Male', 'Female', 'Unisex'),
    belt_length varchar(10),
    belt_width varchar(10),
    isActive boolean default true,
    earningsAddedtoSeller boolean default false,
    FOREIGN KEY FK_SellerUsername (Seller_Username) REFERENCES account(username) ON DELETE CASCADE,
    FOREIGN KEY FK_WinnerUsername (Winner_Username) REFERENCES account(username) ON DELETE CASCADE
);

CREATE TABLE bids (
    Bid_ID int NOT NULL AUTO_INCREMENT PRIMARY KEY, 
    username varchar(50),
    Bid_Amount float DEFAULT NULL,
    Clothing_ID int NOT NULL,   
    Bid_DateTime datetime DEFAULT NULL,
    Max_Bid_Amount float DEFAULT NULL,
    Reached_Limit boolean default false,
    Acknowledged boolean default false, 
    FOREIGN KEY FK_BidsUsername (username) REFERENCES account(username),
    FOREIGN KEY FK_BidsClothingID (Clothing_ID) REFERENCES clothing(Clothing_ID)
);


create table qna(
	qnaid int auto_increment primary key,
    q_username varchar(50),
    q_message varchar(500),
    a_username varchar(50),
    a_message varchar(500),
    foreign key(q_username) references account(username),
    foreign key(a_username) references account(username)
);


CREATE TABLE wishes(
	Wish_ID int NOT NULL AUTO_INCREMENT PRIMARY KEY , 
    Wisher_Username varchar(50),
	Clothing_Category ENUM('Shirt', 'Pant', 'Belt', 'Shoe') NOT NULL, 
	Brand varchar(20),
	Color varchar(15),
	style varchar(15),
	size varchar(5), 
	gender ENUM('Male', 'Female', 'Unisex'), 
	belt_length varchar(10),
	belt_width varchar(10), 
	item_found boolean default false,
    Foreign Key (Wisher_Username) references account(username) on delete cascade
 );

insert into account values ('admin','securepassword','admin@gmail.com','Admin',NULL, false);