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
		String selectedClothingType = request.getParameter("selectedClothingType");
		Class.forName("com.mysql.jdbc.Driver");
	    Connection con = DriverManager.getConnection("jdbc:mysql://localhost:3306/Clothes_to_my_Heart","root", "password");
	    Statement st = con.createStatement();
	    ResultSet detailsRS;
	    detailsRS = st.executeQuery("Select SUM(Current_Bid_Value) AS Total_Current_Bid_Value FROM clothing WHERE isActive = false AND (Reserve_Price IS NULL OR Current_Bid_Value > Reserve_Price) AND Clothing_Category = '" + selectedClothingType + "'");
	    if(detailsRS.next()){
	    	float Total_Current_Bid_Value = detailsRS.getFloat("Total_Current_Bid_Value");
	    	%>
			<p>Earnings of <%= selectedClothingType%> </p>
			<p>Earnings: <%= Total_Current_Bid_Value%>
			<% 
	    }
	%>
	
	
	
	
	

</body>
</html>