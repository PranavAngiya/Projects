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
		String selectedUsername = request.getParameter("selectedUsername");
		%>
		<p>Earnings of <%= selectedUsername%></p>
		<% 
		Class.forName("com.mysql.jdbc.Driver");
	    Connection con = DriverManager.getConnection("jdbc:mysql://localhost:3306/Clothes_to_my_Heart","root", "password");
	    Statement st = con.createStatement();
	    ResultSet detailsRS;
	    detailsRS = st.executeQuery("select earnings from account where username = '" + selectedUsername + "'");
	    if(detailsRS.next()){
	    	float earnings=detailsRS.getFloat("earnings");
	    	%>
	    	<br>
	    	<p>Earnings: <%=earnings %></p>
	
	    	<% 
	    }
	%>
	
	
	
	
	

</body>
</html>