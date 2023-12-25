<%@ page language="java" contentType="text/html; charset=UTF-8"
    pageEncoding="UTF-8"%>
<%@ page import="java.io.*,java.util.*,java.sql.*"%>
<%@ page import="javax.servlet.http.*,javax.servlet.*" %>
<!DOCTYPE html>
<html>
<head>
	<meta charset="UTF-8">
	<title>Home</title>
	<link rel = "stylesheet" type="text/css" href="styles/user_home.css">

</head>
<body>
	<div class = "container">
		<div class = "nav_div">
			<nav>
				<div class="welcome_text">
					<h2>
						Welcome <%=session.getAttribute("user")%> 
					</h2>
					
					 
				</div>
				
				<div class = "nav_buttons">
					<a href = "UserQnA.jsp"><button>Questions and Answers</button></a>
					<a href = "user_profile.jsp"><button>Profile</button></a>
					<a href='logout.jsp'><button>Logout</button></a>
				</div>
					
			</nav>
		</div>
		
		<div class = "Alerts">
			<%
				String userid = (String) session.getAttribute("user");
				Class.forName("com.mysql.jdbc.Driver");
	     	    Connection con = DriverManager.getConnection("jdbc:mysql://localhost:3306/Clothes_to_my_Heart","root", "password");
	     	    Statement st = con.createStatement();
	     	    ResultSet rs;
	     	    rs = st.executeQuery("select * from account where username='" + userid + "' and alerts = true");
	     	    if(rs.next()){
	     	    	out.println("<h2>You have alerts. View Profile to see alerts</h2><br><br>");
	     	    }
				
			%>
			
		
		</div>
		
		<div class = "wish_list">
			<p>View item wish list</p>
			<a href = "user_wish_list.jsp"><Button>View Wish List</Button></a>
			<br><br>
		
		</div>
		
		<div class = "personal_bids">
			<p>View Past Auctions Created</p>
			<a href = "viewpastauctions.jsp"><button>Past Auctions Created</button></a>
			<br><br>
		</div>
		
		<div class = "create_auction">
			<p>Create a New Auction</p>
			<a href = "newauction.jsp"><button>Create New Auction</button></a>
			<br><br>
		</div>
		
		<div class = "search_bids">
			<p>Search for Auctions</p>
			<a href = "viewauctions.jsp"><button>Search Auctions</button></a>
		</div><br>
	
	
	</div>
</body>
</html>