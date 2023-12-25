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
	<a href= "rep_home.jsp"><button>Go Back</button></a>
	<%
		String selectedUsername = request.getParameter("selectedUsername");
		%>
		<p>Details about <%= selectedUsername%></p>
		<% 
		String email;
		String password;
		Class.forName("com.mysql.jdbc.Driver");
	    Connection con = DriverManager.getConnection("jdbc:mysql://localhost:3306/Clothes_to_my_Heart","root", "password");
	    Statement st = con.createStatement();
	    ResultSet detailsRS;
	    detailsRS = st.executeQuery("select * from account where username = '" + selectedUsername + "'");
	    if(detailsRS.next()){
	    	email=detailsRS.getString("email");
	    	password = detailsRS.getString("password");
	    	%>
	    	<br>
	    	<p>Email: <%=email %></p>
	    	<p>Change Email</p>
	    	<form method="post">
	    		New Email: <input type = "text" name = "newemail"/> <br/>
	    		<input type="submit" name = "submit" value="Submit"/>
	    	</form>
	    	
	    	<%
   	
		   	if(request.getParameter("submit") != null){
				String newemail = request.getParameter("newemail"); 
			    
			    if (newemail != null) {
				    ResultSet usernameRS;
				    usernameRS = st.executeQuery("select * from account where email ='" + newemail + "'");
				    if (usernameRS.next()) {
				        out.println("<p>Account already exists with that Email</p>");
				    } 
				    else {
				    	PreparedStatement insertStatement = con.prepareStatement("update account set email = ? where username = ?");
				        insertStatement.setString(1, newemail);
				        insertStatement.setString(2, selectedUsername);
				    	int rowsAffected = insertStatement.executeUpdate();
				    	out.println("<p>Updated Email</p>");
				    }
			    }
		   	}
			%>
	    	
	    	
	    	<br>
	    	<p>Password: <%=password%></p>
	    	<p>Change Password</p>
	    	<form method="post">
	    		New Password: <input type = "text" name = "newpassword"/> <br/>
	    		<input type="submit" name = "submit" value="Submit"/>
	    	</form>
	    	
	    	<%
   	
		   	if(request.getParameter("submit") != null){
				String newpassword = request.getParameter("newpassword"); 
			    
			    if (newpassword != null) {
			    	PreparedStatement insertStatement = con.prepareStatement("update account set password = ? where username = ?");
			        insertStatement.setString(1, newpassword);
			        insertStatement.setString(2, selectedUsername);
			    	int rowsAffected = insertStatement.executeUpdate();
			    	out.println("<p>Updated Password</p>");
				    
			    }
		   	}
			%>
			
			
			
			
			
			<br>
			<p>Delete Account?</p>
			<form method="post">
	    		Type Yes to Delete Password: <input type = "text" name = "confirm"/> <br/>
	    		<input type="submit" name = "submit" value="Submit"/>
	    	</form>
	    	
	    	<%
   	
		   	if(request.getParameter("submit") != null){
				String confirmation = request.getParameter("confirm"); 
			    
			    if (confirmation != null) {
			    	if(confirmation.equals("Yes")){
			    		PreparedStatement insertStatement = con.prepareStatement("delete from account where username = ?");
				        insertStatement.setString(1, selectedUsername);
				    	int rowsAffected = insertStatement.executeUpdate();
				    	out.println("<p>Deleted Account</p>");
			    	}
			    	else{
			    		out.println("Please type Yes to confirm");
			    	}
			    	
				    
			    }
		   	}
			%>
			
	    	
	    	
	    	
	    	
	    	
	    	
	    	
	    	<% 
	    }
	%>
	
	
	
	
	

</body>
</html>