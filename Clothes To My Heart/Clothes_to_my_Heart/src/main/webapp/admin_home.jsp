<%@ page language="java" contentType="text/html; charset=UTF-8"
    pageEncoding="UTF-8"%>
<%@ page import="java.io.*,java.util.*,java.sql.*"%>
<%@ page import="javax.servlet.http.*,javax.servlet.*"%>
<!DOCTYPE html>
<html>
<head>
	<meta charset="UTF-8">
	<title>Admin Home</title>
	<link rel = "stylesheet" type="text/css" href="styles/admin_home.css">
</head>
<body>
	<div class = "container">
		<div class = "nav_div">
			<nav>
				<div class="welcome_text">
					<p>
						Welcome <%=session.getAttribute("user")%> 
					</p>
					
					 
				</div>
				
				<div class = "nav_buttons">
					<a href='logout.jsp'><button>Logout</button></a>
				</div>
					
			</nav>
		</div>
		
		<div class = "create_rep">
			<p>Create Customer Representative Account</p>
			<a href = "createrepaccount.jsp"><button>Create Account</button></a>
		</div>
		
		<br><br>
		
		<div class = "Total_Earnings">
			<p>Total Website Earnings</p>
			<%
				Class.forName("com.mysql.jdbc.Driver");
				Connection con = DriverManager.getConnection("jdbc:mysql://localhost:3306/Clothes_to_my_Heart","root", "password");
				Statement stmt = con.createStatement();
				ResultSet rs;
				
				PreparedStatement statement = con.prepareStatement("Select SUM(Current_Bid_Value) AS Total_Current_Bid_Value FROM clothing WHERE isActive = false AND (Reserve_Price IS NULL OR Current_Bid_Value > Reserve_Price)");
				rs = statement.executeQuery();
				if(rs.next()){
					double totalEarnings = rs.getDouble("Total_Current_Bid_Value");
					%>
					<p>Total: <%=totalEarnings %></p>
					<%
	
				}
			%>
		</div>
		
		<br><br>
		
		<div class = "Best">
			<p>Best Selling item</p>
			<%
				con = DriverManager.getConnection("jdbc:mysql://localhost:3306/Clothes_to_my_Heart","root", "password");
				stmt = con.createStatement();
				
				statement = con.prepareStatement("SELECT Clothing_ID, Clothing_Description, Current_Bid_Value AS Highest_Current_Bid_Value FROM clothing WHERE Current_Bid_Value = ( SELECT MAX(Current_Bid_Value) FROM clothing WHERE isActive = false AND (Reserve_Price IS NULL OR Current_Bid_Value > Reserve_Price))");
				rs = statement.executeQuery();
				if(rs.next()){
					double totalEarnings = rs.getDouble("Highest_Current_Bid_Value");
					int Clothing_ID = rs.getInt("Clothing_ID");
					String Clothing_Description = rs.getString("Clothing_Description");
					%>
					<p>Clothing ID: <%=Clothing_ID %>, Description: <%=Clothing_Description %>, Final Price: <%=totalEarnings %></p>
					<%
				}
			%>
			
			<br>
			<p>Best Auction Hoster</p>
			<%
				con = DriverManager.getConnection("jdbc:mysql://localhost:3306/Clothes_to_my_Heart","root", "password");
				stmt = con.createStatement();
				
				statement = con.prepareStatement("Select username,earnings from account ORDER BY earnings DESC LIMIT 1;");
				rs = statement.executeQuery();
				if(rs.next()){
					String username = rs.getString("username");
					float earnings = rs.getFloat("earnings");
					%>
					<p>Username with the highest earnings: <%=username %>, earnings: <%=earnings %></p>
					<%
				}
			%>
			
		</div>
		
		<br><br>
		
		<div class = "Earnings_per">
			<p>Earnings per item</p>
			<form action = "ItemEarnings.jsp" method = "get">
				<select name = "selectedClothingID">
					<option value = "">Select a Item Description</option>
					<%
						con = DriverManager.getConnection("jdbc:mysql://localhost:3306/Clothes_to_my_Heart","root", "password");
					    Statement st = con.createStatement();
					    ResultSet Clothing;
					    
					    Clothing = st.executeQuery("Select Clothing_ID, Clothing_Description FROM clothing WHERE isActive = false AND (Reserve_Price IS NULL OR Current_Bid_Value > Reserve_Price);");
					    
					    while(Clothing.next()){
					    	String ClothingDescription = Clothing.getString("Clothing_Description");
					    	int Clothing_ID = Clothing.getInt("Clothing_ID");
					    	%>
					    	<option value="<%= Clothing_ID %>"><%= ClothingDescription %></option>
                        <%
					    	
					    }
					%>
					
				</select>
				<input type = "submit" value = "Show Details">
			</form>
			<br>
			
			<p>Earnings per item type</p>
			<form action = "ItemTypeEarnings.jsp" method = "get">
				<select name = "selectedClothingType">
					<option value = "">Select a Item Type</option>
					<option value="Shirt">Shirt</option>
					<option value="Pant">Pant</option>
					<option value="Belt">Belt</option>
					<option value="Shoe">Shoe</option>
				</select>
				<input type = "submit" value = "Show Details">
			</form>
			<br>
			
			<p>Earnings per user</p>
			<form action = "UserEarnings.jsp" method = "get">
				<select name = "selectedUsername">
					<option value = "">Select a Username</option>
					<%
						con = DriverManager.getConnection("jdbc:mysql://localhost:3306/Clothes_to_my_Heart","root", "password");
					    st = con.createStatement();
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
		
		<br><br>
		
		
	
	
	</div>
</body>
</html>