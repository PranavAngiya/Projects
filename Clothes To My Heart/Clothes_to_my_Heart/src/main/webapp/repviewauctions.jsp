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
Click on any auction to view more details or to place a bid.
<br>
<%
    Connection con = null;
    Statement stmt = null;
    ResultSet rs = null;

    try {
        Class.forName("com.mysql.jdbc.Driver");
        con = DriverManager.getConnection("jdbc:mysql://localhost:3306/Clothes_to_my_Heart", "root", "password");
        stmt = con.createStatement();
        
        String updateSQL = "UPDATE clothing SET isActive = 0 WHERE Closing_Date_Time < NOW() AND isActive = 1";
        stmt.executeUpdate(updateSQL);

        String query = "SELECT Clothing_ID, Seller_Username, Clothing_Category, Brand, Color, Current_Bid_Value, Closing_Date_Time, Clothing_Description, isActive FROM clothing";
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
        <td><a href="repauctionDetails.jsp?clothingID=<%= rs.getInt("Clothing_ID") %>"><%= rs.getString("Clothing_Category") %></a></td>
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

<a href="rep_home.jsp"><button>Back to Home</button></a>

</body>
</html>
