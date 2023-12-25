<%@ page language="java" contentType="text/html; charset=ISO-8859-1"
	pageEncoding="ISO-8859-1" %>
<!--Import some libraries that have classes that we need -->
<%@ page import="java.io.*,java.util.*,java.sql.*"%>
<%@ page import="javax.servlet.http.*,javax.servlet.*"%>
<!DOCTYPE html>
<html>
<head>
	<meta http-equiv="Content-Type" content="text/html; charset=ISO-8859-1">
	<title>New Auction Created</title>
	<link rel = "stylesheet" type="text/css" href="styles/welcome.css">
</head>
<body>
<% 
	try{
		
		Class.forName("com.mysql.jdbc.Driver");
		Connection con = DriverManager.getConnection("jdbc:mysql://localhost:3306/Clothes_to_my_Heart","root", "password");
		//Create a SQL statement
		Statement stmt = con.createStatement();
		ResultSet rs;

		String username = (String) session.getAttribute("user");
		String category = request.getParameter("category");
		String clothingBrand = request.getParameter("clothing_brand");
		String color = request.getParameter("color");
		String description = request.getParameter("description");
		float initial_bid_value = Float.valueOf(request.getParameter("initial_bid_value"));
		String date_time = request.getParameter("date") + " " + request.getParameter("time");
		String min_increment = request.getParameter("min_increment");
		String reserve_price = request.getParameter("reserve_price");
		PreparedStatement ps;
		if (category.equals("Belt")){
			String belt_length = request.getParameter("belt_length");
			String belt_width = request.getParameter("belt_width");
			String insert = "INSERT INTO clothing(Seller_Username, Clothing_Category, Brand, Color, Clothing_Description, Current_Bid_Value, Closing_Date_Time, Min_Bid_Increment, Reserve_Price, belt_length, belt_width)"
					+ "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";
			ps = con.prepareStatement(insert);
			ps.setString(10, belt_length);
			ps.setString(11, belt_width);
		}
		
		else{
			String clothing_style = request.getParameter("clothing_style");
			String size = request.getParameter("size");
			String gender = request.getParameter("gender");
			String insert = "INSERT INTO clothing(Seller_Username, Clothing_Category, Brand, Color, Clothing_Description, Current_Bid_Value, Closing_Date_Time, Min_Bid_Increment, Reserve_Price, style, size, gender)"
					+ "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";
			ps = con.prepareStatement(insert);
			ps.setString(10, clothing_style);
			ps.setString(11, size);
			ps.setString(12, gender);
		}
		
		ps.setString(1, username);
		ps.setString(2, category);
		ps.setString(3, clothingBrand);
		ps.setString(4, color);
		ps.setString(5, description);
		ps.setFloat(6, initial_bid_value);
		ps.setString(7, date_time);
		ps.setString(8, min_increment);
		
		if (reserve_price == null || reserve_price.trim().isEmpty()) {
		    // Handle the empty value, for example:
		    ps.setNull(9, java.sql.Types.FLOAT);  
		} else {
		    float reservePriceValue = Float.valueOf(reserve_price);
		    ps.setFloat(9, reservePriceValue);
		}

		ps.executeUpdate();
				
		
		con.close();
				
	}

	finally{
		
		out.print("Auction created!");
	
		%>
		<a href="newauction.jsp"><button>Create Another Auction</button></a>
		<a href="user_home.jsp"><button>Home</button></a>
		<%
	}

%>
		
</body>
</html>