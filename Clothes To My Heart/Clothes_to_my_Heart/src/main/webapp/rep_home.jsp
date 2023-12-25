<%@ page language="java" contentType="text/html; charset=UTF-8" pageEncoding="UTF-8"%>
<%@ page import="java.io.*,java.util.*,java.sql.*"%>
<%@ page import="javax.servlet.http.*,javax.servlet.*"%>
<!DOCTYPE html>
<html>
<head>
	<meta charset="UTF-8">
	<title>Customer Representative Home</title>
	<link rel = "stylesheet" type="text/css" href="styles/user_home.css">
</head>
<body>

	<div class = "container">
		<div class = "nav_div">
			<nav>
				<div class="welcome_text">
					<p>
						Welcome Customer Representative <%=session.getAttribute("user")%> 
					</p>
					
					 
				</div>
				
				<div class = "nav_buttons">
					<a href = 'RepQnA.jsp'><button>Questions and Answers</button></a>
					<a href='logout.jsp'><button>Logout</button></a>
				</div>
					
			</nav>
		</div>
		
		<div class = "users_list">
			<p>View List of Users</p>
			<form action = "UserDetails.jsp" method = "get">
				<select name = "selectedUsername">
					<option value = "">Select a Username</option>
					<%
						Class.forName("com.mysql.jdbc.Driver");
					    Connection con = DriverManager.getConnection("jdbc:mysql://localhost:3306/Clothes_to_my_Heart","root", "password");
					    Statement st = con.createStatement();
					    ResultSet usernameRS;
					    
					    usernameRS = st.executeQuery("select username from account where acctype = 'End_User'");
					    
					    while(usernameRS.next()){
					    	String username = usernameRS.getString("username");
					    	%>
					    	<option value="<%= username %>"><%= username %></option>
                        <%
					    	
					    }
					%>
					
				</select>
				<input type = "submit" value = "Show Details">
			</form>
			
		</div>
		
		<p>View all Auctions</p>
		<a href = "repviewauctions.jsp"><button>View Auctions</button></a>
		
		
		
		
		
		
		
	
	</div>



	
</body>
</html>