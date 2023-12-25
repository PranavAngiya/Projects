<%@ page language="java" contentType="text/html; charset=UTF-8"
    pageEncoding="UTF-8"%>
    <%@ page import="java.io.*,java.util.*,java.sql.*"%>
<%@ page import="javax.servlet.http.*,javax.servlet.*"%>
<!DOCTYPE html>
<html>
<head>
	<meta charset="UTF-8">
	<title>Create Customer Representative Account</title>
	<link rel = "stylesheet" type="text/css" href="styles/welcome.css">
</head>
<body>
	<a href = "admin_home.jsp"><button>Go Back</button></a>
	<form method="POST" class = "create_form">
     	Username: <input type="text" name="username"/> <br/>
     	Email: <input type = "text" name = "email"/> <br/>
     	Password: <input type="password" name="password"/> <br/>
     	<input type="submit" name = "submit" value="Submit"/>
   	</form>	
   	
   	<%
   	
   	if(request.getParameter("submit") != null){
	    String userid = request.getParameter("username");   
		String email = request.getParameter("email"); 
	    String pwd = request.getParameter("password");
	    
	    if (userid != null && email != null && pwd != null) {
		    Class.forName("com.mysql.jdbc.Driver");
		    Connection con = DriverManager.getConnection("jdbc:mysql://localhost:3306/Clothes_to_my_Heart","root", "password");
		    Statement st = con.createStatement();
		    ResultSet usernameRS;
		    usernameRS = st.executeQuery("select * from account where username='" + userid + "' or email ='" + email + "'");
		    if (usernameRS.next()) {
		        out.println("<p>Account already exists with that Username or Email</p>");
		    } 
		    else {
		    	PreparedStatement insertStatement = con.prepareStatement("INSERT INTO account (username, password, email,acctype) VALUES (?, ?, ?,'Customer_Representative')");
		        insertStatement.setString(1, userid);
		        insertStatement.setString(2, pwd);
		        insertStatement.setString(3, email);
		    	int rowsAffected = insertStatement.executeUpdate();
		    	response.sendRedirect("admin_home.jsp");
		    }
	    }
   	}
%>
</body>
</html>