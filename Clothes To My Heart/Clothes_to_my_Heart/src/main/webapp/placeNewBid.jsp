<%@ page language="java" contentType="text/html; charset=ISO-8859-1" pageEncoding="ISO-8859-1" %>
<%@ page import="java.io.*,java.util.*,java.sql.*"%>
<%@ page import="javax.servlet.http.*,javax.servlet.*"%>
<%@ page import="java.text.DecimalFormat" %>

<html>
<head>
    <title>Place New Bid</title>
	<link rel = "stylesheet" type="text/css" href="styles/welcome.css">
</head>
<body>
<h2>Place New Bid</h2>

<%
    Connection con = null;
    PreparedStatement pstmt = null;
    ResultSet rs = null;
    int clothingID = Integer.parseInt(request.getParameter("clothingID"));
    float currentBidValue = 0;
    float minBidIncrement = 0;
    
    try {
        Class.forName("com.mysql.jdbc.Driver");
        con = DriverManager.getConnection("jdbc:mysql://localhost:3306/Clothes_to_my_Heart", "root", "password");
        
        String query = "SELECT Current_Bid_Value, Min_Bid_Increment FROM clothing WHERE Clothing_ID=?";
        pstmt = con.prepareStatement(query);
        pstmt.setInt(1, clothingID);
        rs = pstmt.executeQuery();

        if(rs.next()) {
            currentBidValue = rs.getFloat("Current_Bid_Value");
            minBidIncrement = rs.getFloat("Min_Bid_Increment");
        }
%>
        Current Bid Value: $<%= new DecimalFormat("###,###.00").format(currentBidValue) %><br>
        Minimum Next Bid: $<%= new DecimalFormat("###,###.00").format(currentBidValue + minBidIncrement) %><br>

        
           
<%
	String username = (String) session.getAttribute("user");
    boolean hasSetAutoBid = false;
    String checkAutoBid = "SELECT * FROM bids WHERE username=? AND Clothing_ID=? AND Max_Bid_Amount IS NOT NULL";
    pstmt = con.prepareStatement(checkAutoBid);
    pstmt.setString(1, username);
    pstmt.setInt(2, clothingID);
    rs = pstmt.executeQuery();
    if(rs.next()) {
        hasSetAutoBid = true;
    }
%>

<% if (hasSetAutoBid) { %>
    <p>You've already set an automatic bid for this item. </p>
<% } else { %>
<form action="processBid.jsp" method="post">
            Enter Bid Amount: <input type="text" name="bidAmount" required><br>
            <br>
    If you would like to setup automatic bidding, enter maximum bid limit here:
    <input type="text" name="Max_Bid_Amount">
<% } %>
            <input type="hidden" name="clothingID" value="<%= clothingID %>"><br>
            <br>
            <input type="submit" value="Submit">
        </form>
<%
    } catch(Exception e) {
        out.print("Error: " + e.getMessage());
    } finally {
        try {
            if(rs != null) rs.close();
            if(pstmt != null) pstmt.close();
            if(con != null) con.close();
        } catch(SQLException se) {
            out.print("Error closing resources: " + se.getMessage());
        }
    }
%>

</body>
</html>
