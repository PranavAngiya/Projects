<%@ page language="java" contentType="text/html; charset=UTF-8"
    pageEncoding="UTF-8"%>
<%@ page import="java.io.*,java.util.*,java.sql.*"%>
<%@ page import="javax.servlet.http.*,javax.servlet.*"%>
<!DOCTYPE html>
<html>
<head>
	<meta charset="UTF-8">
	<title>Create Wish List</title>
	<link rel = "stylesheet" type="text/css" href="styles/welcome.css">
</head>
<body>

<% 
	try{
		
		Class.forName("com.mysql.jdbc.Driver");
		Connection con = DriverManager.getConnection("jdbc:mysql://localhost:3306/Clothes_to_my_Heart","root", "password");
		Statement stmt = con.createStatement();
		ResultSet rs;

		String category = request.getParameter("category");
		String clothingBrand = request.getParameter("clothing_brand");
		String color = request.getParameter("color");
		PreparedStatement ps;
		if (category.equals("Belt")){
			String belt_length = request.getParameter("belt_length");
			String belt_width = request.getParameter("belt_width");
			String insert = "INSERT INTO wishes(Wisher_Username, Clothing_Category, Brand, Color, belt_length, belt_width)"
					+ "VALUES (?, ?, ?, ?, ?, ?)";
			ps = con.prepareStatement(insert);
			ps.setString(5, belt_length);
			ps.setString(6, belt_width);
		}
		
		else{
			String clothing_style = request.getParameter("clothing_style");
			String size = request.getParameter("size");
			String gender = request.getParameter("gender");
			String insert = "INSERT INTO wishes(Wisher_Username, Clothing_Category, Brand, Color, style, size, gender)"
					+ "VALUES (?, ?, ?, ?, ?, ?, ?)";
			ps = con.prepareStatement(insert);
			ps.setString(5, clothing_style);
			ps.setString(6, size);
			ps.setString(7, gender);
		}
		
		ps.setString(1, (String) session.getAttribute("user") );
		ps.setString(2, category);
		ps.setString(3, clothingBrand);
		ps.setString(4, color);

		ps.executeUpdate();
				
		
		con.close();
				
	}

	finally{
		
		out.print("Wish created!");
	
		%>
		<a href="user_wish_list.jsp"><button>Create Another Wish</button></a>
		<a href="user_home.jsp"><button>Home</button></a>
		<%
	}

%>



</body>
</html>