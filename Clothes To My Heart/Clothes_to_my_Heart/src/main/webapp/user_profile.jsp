<%@ page language="java" contentType="text/html; charset=UTF-8"
    pageEncoding="UTF-8"%>
<%@ page import="java.io.*,java.util.*,java.sql.*"%>
<%@ page import="javax.servlet.http.*,javax.servlet.*"%>
<!DOCTYPE html>
<html>
<head>
	<meta charset="UTF-8">
	<title>User Profile</title>
	<link rel = "stylesheet" type="text/css" href="styles/user_home.css">
</head>

<body>
	<a href = "user_home.jsp"><button>Go Back</button></a>
	
	<h2>Alerts</h2>
	
	
	<%
		String userid = (String) session.getAttribute("user");
		Class.forName("com.mysql.jdbc.Driver");
	    Connection con = DriverManager.getConnection("jdbc:mysql://localhost:3306/Clothes_to_my_Heart","root", "password");
	    Statement st = con.createStatement();
	    ResultSet rs;
	    rs = st.executeQuery("select * from account where username='" + userid + "' and alerts = true");
	    if(rs.next()){
	    	rs = st.executeQuery("select * from clothing where Winner_Username = '"+userid +"' and Winner_Acknowledged = false");
	    	if(rs.next()){
	    		out.println("<p>You have won an auction. Click the button to acknowledge the alert</p>");
	    		%>
	    		<form method="post">
	    			<input type="submit" value = "Acknowledge" name = "acknowledge">
	    		</form>
	    		
	    		<%
	    			if(request.getParameter("acknowledge") != null){
	    				PreparedStatement updater;
		            	updater = con.prepareStatement("update account set alerts = false where username = ?");
		            	updater.setString(1,userid);
		            	updater.executeUpdate();
		            	updater.close();
		            	con.close();
		            	con = DriverManager.getConnection("jdbc:mysql://localhost:3306/Clothes_to_my_Heart","root", "password");
		            	PreparedStatement clothing_updater = con.prepareStatement("update clothing set Winner_Acknowledged = true where Winner_Username = ?");
		            	clothing_updater.setString(1,userid);
		            	clothing_updater.executeUpdate();
		            	out.println("Alert has been acknowledged");
	    			}
	    		
	    		%>
	    		
	    		<%
	    	}
	
	    }
	    
	    
	    
		Class.forName("com.mysql.jdbc.Driver");
	    Connection con2 = DriverManager.getConnection("jdbc:mysql://localhost:3306/Clothes_to_my_Heart","root", "password");
	    Statement st2 = con.createStatement();
    	rs = st2.executeQuery("select * from bids where username = '"+userid +"' and Reached_Limit = true and Acknowledged = false");
    	if(rs.next()){
    		%>
    		<p>Your upper limit for your automatic bidding has been reached. Click the button to acknowledge the alert</p>
    		<form method="post">
    			<input type="submit" value = "Bid_Acknowledge" name = "Bid_Acknowledge">
    		</form>
    		
    		<%
    			if(request.getParameter("Bid_Acknowledge") != null){
    				PreparedStatement updater;
	            	updater = con2.prepareStatement("update account set alerts = false where username = ?");
	            	updater.setString(1,userid);
	            	updater.executeUpdate();
	            	updater.close();
	            	con2.close();
	            	con2 = DriverManager.getConnection("jdbc:mysql://localhost:3306/Clothes_to_my_Heart","root", "password");
	            	PreparedStatement clothing_updater = con2.prepareStatement("update bids set Acknowledged = true where username = ?");
	            	clothing_updater.setString(1,userid);
	            	clothing_updater.executeUpdate();
	            	out.println("Alert has been acknowledged");
    			}
    		
    		%>
    		
    		<%
    	}
	
	    
	    
	    
	    
	    Connection con1 = DriverManager.getConnection("jdbc:mysql://localhost:3306/Clothes_to_my_Heart","root", "password");
 	    Statement st1 = con1.createStatement();
        ResultSet Check = st1.executeQuery("select * from wishes where Wisher_Username = '" + userid + "' and item_found = true and Acknowledged = false");
        if(Check.next()){
        	String Clothing_Category = Check.getString("Clothing_Category");
        	String Brand = Check.getString("Brand");
        	String Color = Check.getString("Color");
        	String style = Check.getString("style");
        	String size = Check.getString("size");
        	String gender = Check.getString("gender");
        	String belt_length = Check.getString("belt_length");
        	String belt_width = Check.getString("belt_width");
        	
        	String query = "SELECT * from clothing where 1=1";
        	
        	if(Clothing_Category != null){
        		query += " AND Clothing_Category = '" + Clothing_Category + "'";
        	}
        	if(Brand != null){
        		query += " AND Brand = '" + Brand + "'";
        	}
        	if(Color != null){
        		query += " AND Color = '" + Color + "'";
        	}
        	if(style != null){
        		query += " AND style = '" + style + "'";
        	}
        	if(size != null){
        		query += " AND size = '" + size + "'";
        	}
        	if(gender != null){
        		query += " AND gender = '" + gender + "'";
        	}
        	if(belt_length != null){
        		query += " AND belt_length = '" + belt_length + "'";
        	}
        	if(belt_width != null){
        		query += " AND belt_width = '" + belt_width + "'";
        	}
        	
        	ResultSet match = st.executeQuery(query);
        	if(match.next()){
        		out.println("<p>Your Wish List Item has been found. Click the button to acknowledge the alert</p>");
	    		%>
	    		<form method="post">
	    			<input type="submit" value = "Acknowledge" name = "acknowledge">
	    		</form>
	    		
	    		<%
	    			if(request.getParameter("acknowledge") != null){
	    				PreparedStatement updater;
		            	updater = con.prepareStatement("update account set alerts = false where username = ?");
		            	updater.setString(1,userid);
		            	updater.executeUpdate();
		            	updater.close();
		            	con.close();
		            	con = DriverManager.getConnection("jdbc:mysql://localhost:3306/Clothes_to_my_Heart","root", "password");
		            	PreparedStatement clothing_updater = con.prepareStatement("update wishes set Acknowledged = true where Wisher_Username = ?");
		            	clothing_updater.setString(1,userid);
		            	clothing_updater.executeUpdate();
		            	out.println("Alert has been acknowledged");
	    			}
	    		
	    		%>
	    		
	    		<%
        	}
        	
        }
	    
	    
	    
	    
	    
	    
	    
	    
	    
	%>
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	

</body>
</html>