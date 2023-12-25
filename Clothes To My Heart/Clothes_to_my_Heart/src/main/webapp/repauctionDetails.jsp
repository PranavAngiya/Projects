<%@ page language="java" contentType="text/html; charset=ISO-8859-1"
    pageEncoding="ISO-8859-1" %>
<%@ page import="java.io.*,java.util.*,java.sql.*"%>
<%@ page import="javax.servlet.http.*,javax.servlet.*"%>
<%@ page import="java.text.DecimalFormat, java.text.SimpleDateFormat" %>

<html>
<head>
    <title>Auction Details</title>
	<link rel = "stylesheet" type="text/css" href="styles/welcome.css">
</head>
<body>
<a href="repviewauctions.jsp"><button>Back to Auctions List</button></a>
<h2>Auction Details</h2>

			<p>Delete Auction?</p>
			<form method="post">
	    		Type Yes to Delete Auction: <input type = "text" name = "confirm"/> <br/>
	    		<input type="submit" name = "submit" value="Submit"/>
	    	</form>
	    	
	    	<%
   	
		   	if(request.getParameter("submit") != null){
		   		int clothingID = Integer.parseInt(request.getParameter("clothingID"));
				String confirmation = request.getParameter("confirm"); 
			    
			    if (confirmation != null) {
			    	if(confirmation.equals("Yes")){
			    		Class.forName("com.mysql.jdbc.Driver");
			            Connection con = DriverManager.getConnection("jdbc:mysql://localhost:3306/Clothes_to_my_Heart", "root", "password");
			    		PreparedStatement insertStatement = con.prepareStatement("delete from clothing where Clothing_ID = ?");
				        insertStatement.setInt(1, clothingID);
				    	int rowsAffected = insertStatement.executeUpdate();
				    	out.println("<p>Deleted Auction</p>");
			    	}
			    	else{
			    		out.println("Please type Yes to confirm");
			    	}
			    	
				    
			    }
		   	}
			%>


<%
    Connection con = null;
    PreparedStatement pstmt = null;
    PreparedStatement psBid = null;
    ResultSet rs = null;
    ResultSet rsBids = null;

    int clothingID = Integer.parseInt(request.getParameter("clothingID"));

    try {
        Class.forName("com.mysql.jdbc.Driver");
        con = DriverManager.getConnection("jdbc:mysql://localhost:3306/Clothes_to_my_Heart", "root", "password");
        
        String query = "SELECT Clothing_Category, Brand, Color, Current_Bid_Value, Closing_Date_Time, Clothing_Description, isActive, Style, Size, Gender, Belt_Length, Belt_Width FROM clothing WHERE Clothing_ID=?";
        pstmt = con.prepareStatement(query);
        pstmt.setInt(1, clothingID);
        rs = pstmt.executeQuery();

        if(rs.next()) {
%>
    <ul>
        <li>Clothing Category: <%= rs.getString("Clothing_Category") %></li>
        <li>Brand: <%= rs.getString("Brand") %></li>
        <li>Color: <%= rs.getString("Color") %></li>
        <li>Current Bid Value: <%= new DecimalFormat("$###,###.00").format(rs.getFloat("Current_Bid_Value")) %></li>
        <li>Closing Date and Time: <%= rs.getString("Closing_Date_Time") %></li>
        <li>Clothing Description: <%= rs.getString("Clothing_Description") %></li>
        <li>Is Active: <%= rs.getBoolean("isActive") ? "Yes" : "No" %></li>
        <% if(rs.getString("Style") != null) { %>
            <li>Style: <%= rs.getString("Style") %></li>
        <% } %>
        <% if(rs.getString("Size") != null) { %>
            <li>Size: <%= rs.getString("Size") %></li>
        <% } %>
        <% if(rs.getString("Gender") != null) { %>
            <li>Gender: <%= rs.getString("Gender") %></li>
        <% } %>
        <% if(rs.getObject("Belt_Length") != null) { %>
            <li>Belt Length: <%= rs.getString("Belt_Length") %></li>
        <% } %>
        <% if(rs.getObject("Belt_Width") != null) { %>
            <li>Belt Width: <%= rs.getString("Belt_Width") %></li>
        <% } %>
    </ul>
<%
        } else {
            out.print("No auction details found for the provided ID.");
        }

        // Fetching bid history 
        String bidQuery = "SELECT username, Bid_Amount, Bid_DateTime FROM bids WHERE Clothing_ID = ? ORDER BY Bid_DateTime DESC";
        psBid = con.prepareStatement(bidQuery);
        psBid.setInt(1, clothingID);
        rsBids = psBid.executeQuery();
%>

<% if(rsBids != null) { %>
    <h3>Bid History</h3>
    <table border="1">
        <thead>
            <tr>
                <th>Username</th>
                <th>Bid Amount</th>
                <th>Bid Date and Time</th>
            </tr>
        </thead>
        <tbody>
            <% while (rsBids.next()) { %>
            <tr>
                <td><%= rsBids.getString("username") %></td>
                <td><%= new DecimalFormat("$###,###.00").format(rsBids.getFloat("Bid_Amount")) %></td>
                <td><%= new SimpleDateFormat("yyyy-MM-dd HH:mm:ss").format(rsBids.getTimestamp("Bid_DateTime")) %></td>
            </tr>
            <% } %>
        </tbody>
    </table>
    
    
    
    
    <p>Delete most recent bid?</p>
	<form method="post">
   		Type Yes to Delete Recent Bid: <input type = "text" name = "confirm"/> <br/>
   		<input type="submit" name = "submitBid" value="Submit"/>
   	</form>
   	<%
   	if(request.getParameter("submitBid") != null){
		String confirmation = request.getParameter("confirm"); 
	    
	    if (confirmation != null) {
	    	if(confirmation.equals("Yes")){
	    		
	    		//extract the second highest bid value
	    		int deleteBidID;
	    		float replaceAmount;
	    		PreparedStatement extraction = con.prepareStatement("SELECT (SELECT Bid_ID FROM bids WHERE Clothing_ID = ? ORDER BY Bid_Amount DESC LIMIT 1) AS Highest_Bid_ID, (SELECT Bid_Amount FROM bids WHERE Clothing_ID = ? ORDER BY Bid_Amount DESC LIMIT 1 OFFSET 1) AS Second_Highest_Bid_Amount");
	    		extraction.setInt(1,clothingID);
	    		extraction.setInt(2,clothingID);
	    		ResultSet extracted = extraction.executeQuery();
	    		if(extracted.next()){
	    			deleteBidID = extracted.getInt("Highest_Bid_ID");
	    			replaceAmount = extracted.getFloat("Second_Highest_Bid_Amount");
	    			
	    			//delete highest bid in table
	    			PreparedStatement removeBid = con.prepareStatement("delete from bids where Bid_ID = ?");
		    		removeBid.setInt(1,deleteBidID);
		    		int rowsAffected = removeBid.executeUpdate();
		    		
		    		//update Current_Bid_Value
		    		PreparedStatement changeValue = con.prepareStatement("update clothing set Current_Bid_Value = ? where Clothing_ID = ?");
		    		changeValue.setFloat(1,replaceAmount);
		    		changeValue.setInt(2,clothingID);
		    		rowsAffected = changeValue.executeUpdate();
		    		
		    		%>
		    		<p>Bid has been removed</p>
		    		<% 
		    		
	    		}
	    		
	    		
	    		
	    		
	    		
	    		
	    		
	    		
		    	
		    	
		    	
	    	}
	    	else{
	    		out.println("Please type Yes to confirm");
	    	}
	    }
   	}
	%>
    
    
    
    
    
    
    
			
<% } else { %>
    <h3>No bid history available for this item.</h3>
<% } 

    } catch(Exception e) {
        out.print("Error: " + e.getMessage());
    } finally {
        try {
            if(rs != null) rs.close();
            if(rsBids != null) rsBids.close();
            if(pstmt != null) pstmt.close();
            if(psBid != null) psBid.close();
            if(con != null) con.close();
        } catch(SQLException se) {
            out.print("Error closing resources: " + se.getMessage());
        }
    }
%>

	

<br>
<br>



</body>
</html>
