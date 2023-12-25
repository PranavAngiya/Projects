<%@ page language="java" contentType="text/html; charset=ISO-8859-1"
    pageEncoding="ISO-8859-1" %>
<%@ page import="java.io.*,java.util.*,java.sql.*"%>
<%@ page import="javax.servlet.http.*,javax.servlet.*" %>


<!DOCTYPE html>
<html>
   <head>
   		<link rel = "stylesheet" type="text/css" href="styles/welcome.css">
      <title>Welcome</title>
   </head>
   
   <body>
   <h1>Welcome to Clothes to my Heart</h1>
	   <div class = "container">
	   		<div class = "center-box">
	   			<form method="POST" class = "login_form">
			       	Username: <input type="text" name="username"/> <br/>
			       	Password: <input type="password" name="password"/> <br/>
		       	<input type="submit" name = "submit" value="Submit"/>
		     	</form>	
		     	
		     	<%
		     	if(request.getParameter("submit") != null){
		     		String userid = request.getParameter("username");   
		     	    String pwd = request.getParameter("password");
		     	    
		     	   if (userid != null && pwd != null) {
			     	    Class.forName("com.mysql.jdbc.Driver");
			     	    Connection con = DriverManager.getConnection("jdbc:mysql://localhost:3306/Clothes_to_my_Heart","root", "password");
			     	    Statement st = con.createStatement();
			     	    ResultSet rs;
			     	    rs = st.executeQuery("select * from account where username='" + userid + "' and password='" + pwd + "'");
			     	    if (rs.next()) {
			     	        session.setAttribute("user", userid); 
			     	        String accType = rs.getString("acctype");
			     	        if (accType.equals("End_User")){
			     	        	
			     	        	rs = st.executeQuery("select * from clothing where Winner_Username = '"+userid +"' and Winner_Acknowledged = false");
					            if(rs.next()){
					            	PreparedStatement updater;
					            	updater = con.prepareStatement("update account set alerts = true where username = ?");
					            	updater.setString(1,userid);
					            	updater.executeUpdate();
					            }
					            rs.close();
					            
					            
					            Connection con1 = DriverManager.getConnection("jdbc:mysql://localhost:3306/Clothes_to_my_Heart","root", "password");
					     	    Statement st1 = con1.createStatement();
					            ResultSet Check = st1.executeQuery("select * from wishes where Wisher_Username = '" + userid + "' and item_found = false");
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
					            		PreparedStatement updater;
						            	updater = con.prepareStatement("update wishes set item_found = true where Wisher_Username = ?");
						            	updater.setString(1,userid);
						            	updater.executeUpdate();
						            	
					            		PreparedStatement updater1;
						            	updater1 = con.prepareStatement("update account set alerts = true where username = ?");
						            	updater1.setString(1,userid);
						            	updater1.executeUpdate();
					            	}
					            	
					            }
			     	        	
			     	        	
			     	        	
			     	        	
			     	        	response.sendRedirect("user_home.jsp");
			     	        }
			     	        else if(accType.equals("Customer_Representative")){
			     	        	response.sendRedirect("rep_home.jsp");
			     	        }
			     	        else if(accType.equals("Admin")){
			     	        	response.sendRedirect("admin_home.jsp");
			     	        }
			     	        
			     	    } else {
			     	        out.println("<p>Invalid Login Details</p>");
			     	    }
		     	   	}
		     		
		     	}
		     	%>
		     	
		     	<p>Don't have an account?</p>
		     	<a href = "createaccount.jsp"><button >Create Account</button></a>
	   		</div>
		   	
	   </div>
     
   </body>
</html>