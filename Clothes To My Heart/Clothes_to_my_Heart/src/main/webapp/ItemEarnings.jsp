<%@ page language="java" contentType="text/html; charset=UTF-8"
    pageEncoding="UTF-8"%>
<%@ page import="java.io.*,java.util.*,java.sql.*"%>
<%@ page import="javax.servlet.http.*,javax.servlet.*"%>
<!DOCTYPE html>
<html>
<head>
	<meta charset="UTF-8">
	<title>User Details</title>
	<link rel = "stylesheet" type="text/css" href="styles/welcome.css">
</head>
<body>
	<a href= "admin_home.jsp"><button>Go Back</button></a>
	<%
		String selectedClothingID = request.getParameter("selectedClothingID");
		Class.forName("com.mysql.jdbc.Driver");
	    Connection con = DriverManager.getConnection("jdbc:mysql://localhost:3306/Clothes_to_my_Heart","root", "password");
	    Statement st = con.createStatement();
	    ResultSet detailsRS;
	    detailsRS = st.executeQuery("select Clothing_Description, Current_Bid_Value from clothing where Clothing_ID = '" + selectedClothingID + "'");
	    if(detailsRS.next()){
	    	float Current_Bid_Value = detailsRS.getFloat("Current_Bid_Value");
	    	String Clothing_Description = detailsRS.getString("Clothing_Description");
	    	%>
			<p>Earnings of <%= Clothing_Description%> with Clothing ID: <%= selectedClothingID%></p>
			<p>Earnings: <%= Current_Bid_Value%>
			<% 
	    }
	%>
	
	
	
	
	

</body>
</html>