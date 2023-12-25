<%@ page language="java" contentType="text/html; charset=ISO-8859-1"
    pageEncoding="ISO-8859-1" %>
<%@ page import="java.io.*,java.util.*,java.sql.*"%>
<%@ page import="javax.servlet.http.*,javax.servlet.*"%>
<%@ page import="java.text.DecimalFormat" %>

<html>
<head>
    <title>View All Auctions</title>
	<link rel = "stylesheet" type="text/css" href="styles/welcome.css">
</head>
<body>

<a href="user_home.jsp"><button>Back to Home</button></a><br>
<p>Click on any auction to view more details and to place a bid or filter for specific bids.</p>
<%
    Connection con = null;
    Statement stmt = null;
    ResultSet rs = null;
    
    %>
    
    <form method="post">
	
	Choose the appropriate category:
		<select name="category" size=1>
		 			<option value="" selected disabled>Select a category</option>
					<option value="Shirt">Shirt</option>
					<option value="Pant">Pant</option>
					<option value="Belt">Belt</option>
					<option value="Shoe">Shoe</option>
		</select>
		<br>
	
	<div id="dynamicFields" style="display: none;">
	    <!-- Fields for Belt -->
	    <div id="beltFields" style="display: none;">
	        Belt Length: 
	        <table>
		<tr><td><input type="text" name="belt_length"></td>
	</tr>
	</table>
	Belt Width: 
	<table>
	<tr><td><input type="text" name="belt_width"></td>
	</tr>
	
	</table>
	    </div>
	    
	    <!-- Fields for Shirt -->
	    <div id="shirtFields" style="display: none;">
	      Style:
	        <table>
		<tr><td><input type="text" name="clothing_style"></td>
			</tr>
			</table>
	 Size:
	<table>
	<tr><td><input type="text" name="size"></td>
	</tr>
	</table>
	Gender: 
	<br>
    <table>
		<tr><td><select name="gender" size=1>
		 			<option value="" selected disabled>Gender</option>
					<option value="Female">Female</option>
					<option value="Male">Male</option>
					<option value="Unisex">Unisex</option>
		</select></td>
			</tr>
			</table>
	
	    </div>
	</div>
	
	Enter the brand: 
	<table>
		<tr>    
			<td><input type="text" name="clothing_brand"></td>
		
	</tr>
	</table>
	
	Enter the item's color: 
	 
	 <table>
		<tr>    
			<td><input type="text" name="color"></td>
		
	</tr>
	</table>
	
	Enter the Highest Bid Price: 
	<table>
		<tr>    
			<td><input type="text" name="high_price"></td>
		
	</tr>
	</table>
	
	Choose Auction Status: 
	<select name="status" size=1>
		 			<option value="" selected disabled>Select Status</option>
					<option value="Inactive">Inactive</option>
					<option value="Active">Active</option>
					<option value="">Both</option>
	</select> <br>
	
	<input type="submit" name = "filter_submit" value="Submit">   
	</form>
	
	
	<script>
	    document.querySelector('select[name="category"]').addEventListener('change', function() {
	        // Hide all dynamic fields initially
	        document.getElementById('dynamicFields').style.display = 'none';
	        document.getElementById('beltFields').style.display = 'none';
	        document.getElementById('shirtFields').style.display = 'none';
	        // ...hide other category fields as well...
	
	        // Show fields based on the selected category
	        if (this.value === 'Belt') {
	            document.getElementById('dynamicFields').style.display = 'block';
	            document.getElementById('beltFields').style.display = 'block';
	        } else {
	            document.getElementById('dynamicFields').style.display = 'block';
	            document.getElementById('shirtFields').style.display = 'block';
	        }
	
	    });
	</script>
	
	<%
		String additional = " where 1=1";
		if(request.getParameter("filter_submit")!= null){
			String Clothing_Category = request.getParameter("category");
        	String Brand = request.getParameter("clothing_brand");
        	String Color = request.getParameter("color");
        	String style = request.getParameter("clothing_style");
        	String size = request.getParameter("size");
        	String gender = request.getParameter("gender");
        	String belt_length = request.getParameter("belt_length");
        	String belt_width = request.getParameter("belt_width");
        	String price = request.getParameter("high_price");
        	String status = request.getParameter("status");
        	
        	if(Clothing_Category != null && Clothing_Category != ""){
        		additional += " AND Clothing_Category = '" + Clothing_Category + "'";
        	}
        	if(Brand != null && Brand != ""){
        		additional += " AND Brand = '" + Brand + "'";
        	}
        	if(Color != null && Color != ""){
        		additional += " AND Color = '" + Color + "'";
        	}
        	if(style != null && style != ""){
        		additional += " AND style = '" + style + "'";
        	}
        	if(size != null && size != ""){
        		additional += " AND size = '" + size + "'";
        	}
        	if(gender != null && gender != ""){
        		additional += " AND gender = '" + gender + "'";
        	}
        	if(belt_length != null && belt_length != ""){
        		additional += " AND belt_length = '" + belt_length + "'";
        	}
        	if(belt_width != null && belt_width != ""){
        		additional += " AND belt_width = '" + belt_width + "'";
        	}
        	if(price != null && price != ""){
        		additional += " AND Current_Bid_Value < " + price;
        	}
        	if(status != null && status != ""){
        		if(status.equals("Active")){
        			additional += " AND isActive = 1";
        		}
        		else if(status.equals("Inactive")){
        			additional += " AND isActive = 0";
        		}
        	}
		}
	
	%>
	
    
    <% 

    try {
        Class.forName("com.mysql.jdbc.Driver");
        con = DriverManager.getConnection("jdbc:mysql://localhost:3306/Clothes_to_my_Heart", "root", "password");
        stmt = con.createStatement();
        
        String updateSQL = "UPDATE clothing SET isActive = 0 WHERE Closing_Date_Time < NOW() AND isActive = 1";
        stmt.executeUpdate(updateSQL);

        String query = "SELECT Clothing_ID, Seller_Username, Clothing_Category, Brand, Color, Current_Bid_Value, Closing_Date_Time, Clothing_Description, isActive FROM clothing" + additional;
        
        rs = stmt.executeQuery(query);
%>

<table border="1">
    <tr>
        <th>Seller Username</th>
    	<th>Clothing Category</th>
        <th>Brand</th>
        <th>Color</th>
        <th>Current Bid Value</th>
        <th>Closing Date and Time</th>
        <th>Clothing Description</th>
        <th>Is Active</th>
    </tr>

<%

        DecimalFormat df = new DecimalFormat("$###,###.00"); // this will ensure two decimal places and a dollar sign

        while(rs.next()) {
%>
    <tr>
     	<td><%= rs.getString("Seller_Username") %></td>
        <td><a href="auctionDetails.jsp?clothingID=<%= rs.getInt("Clothing_ID") %>"><button><%= rs.getString("Clothing_Category") %></button></a></td>
        <td><%= rs.getString("Brand") %></td>
        <td><%= rs.getString("Color") %></td>
        <td><%= df.format(rs.getFloat("Current_Bid_Value")) %></td> 
        <td><%= rs.getString("Closing_Date_Time") %></td>
        <td><%= rs.getString("Clothing_Description") %></td>
        <td><%= rs.getBoolean("isActive") ? "Yes" : "No" %></td>
    </tr>
<%
        }
    } catch(Exception e) {
        out.print("Error: " + e.getMessage());
    } finally {
        try {
            if(rs != null) rs.close();
            if(stmt != null) stmt.close();
            if(con != null) con.close();
        } catch(SQLException se) {
            out.print("Error closing resources: " + se.getMessage());
        }
    }
%>
</table>



</body>
</html>
