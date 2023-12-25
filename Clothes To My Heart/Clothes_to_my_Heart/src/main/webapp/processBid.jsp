<%@ page language="java" contentType="text/html; charset=ISO-8859-1" pageEncoding="ISO-8859-1" %>
<%@ page import="java.io.*,java.util.*,java.sql.*"%>
<%@ page import="javax.servlet.http.*,javax.servlet.*"%>

<html>
<head>
    <title>Processing Bid</title>
	<link rel = "stylesheet" type="text/css" href="styles/welcome.css">
</head>
<body>

<%
    Connection con = null;
    PreparedStatement pstmt = null;
    ResultSet rs = null;
    int clothingID = Integer.parseInt(request.getParameter("clothingID"));
    float userBidAmount = Float.parseFloat(request.getParameter("bidAmount"));
    Float userMaxBidAmount = null;

    if(request.getParameter("Max_Bid_Amount") != null && !request.getParameter("Max_Bid_Amount").isEmpty()) {
        userMaxBidAmount = Float.parseFloat(request.getParameter("Max_Bid_Amount"));
    }

    try {
        Class.forName("com.mysql.jdbc.Driver");
        con = DriverManager.getConnection("jdbc:mysql://localhost:3306/Clothes_to_my_Heart", "root", "password");

        // Fetching Min_Bid_Increment
        float minBidIncrement = 0.0f;
        String fetchIncrement = "SELECT Min_Bid_Increment FROM clothing WHERE Clothing_ID=?";
        pstmt = con.prepareStatement(fetchIncrement);
        pstmt.setInt(1, clothingID);
        rs = pstmt.executeQuery();

        if (rs.next()) {
            minBidIncrement = rs.getFloat("Min_Bid_Increment");
        }
        
        // Initial bid placement
        String insertBid = "INSERT INTO bids (username, Bid_Amount, Clothing_ID, Bid_DateTime, Max_Bid_Amount) VALUES (?, ?, ?, NOW(), ?)";
        pstmt = con.prepareStatement(insertBid);
        pstmt.setString(1, (String) session.getAttribute("user"));
        pstmt.setFloat(2, userBidAmount);
        pstmt.setInt(3, clothingID);
        if(userMaxBidAmount != null) {
            pstmt.setFloat(4, userMaxBidAmount);
        } else {
            pstmt.setNull(4, Types.FLOAT);
        }
        pstmt.executeUpdate();

        // Start the auto-bidding loop
        boolean canAutoBid = true;
        String lastBidder = (String) session.getAttribute("user");

        while (canAutoBid) {
            canAutoBid = false;  // Assume no more bids can be placed, we will set it true if we can place an auto-bid
            
            // Fetch the user with the next highest Max_Bid_Amount for auto-bidding
            String autoBidUserQuery = "SELECT username, Max_Bid_Amount FROM bids WHERE Clothing_ID=? AND Max_Bid_Amount IS NOT NULL AND Reached_Limit=0 AND Acknowledged=0 AND username <> ? ORDER BY Max_Bid_Amount DESC LIMIT 1";
            pstmt = con.prepareStatement(autoBidUserQuery);
            pstmt.setInt(1, clothingID);
            pstmt.setString(2, lastBidder);
            rs = pstmt.executeQuery();

            if(rs.next()) {
                String autoBidUser = rs.getString("username");
                float autoBidUserMaxAmount = rs.getFloat("Max_Bid_Amount");
                
                // Check if auto-bid can be placed
                if(autoBidUserMaxAmount >= (userBidAmount + minBidIncrement)) {
                    // Place the auto-bid
                    insertBid = "INSERT INTO bids (username, Bid_Amount, Clothing_ID, Bid_DateTime, Max_Bid_Amount) VALUES (?, ?, ?, NOW(), ?)";
                    pstmt = con.prepareStatement(insertBid);
                    pstmt.setString(1, autoBidUser);
                    pstmt.setFloat(2, userBidAmount + minBidIncrement);  // Incremented by minBidIncrement
                    pstmt.setInt(3, clothingID);
                    pstmt.setFloat(4, autoBidUserMaxAmount);
                    pstmt.executeUpdate();

                    userBidAmount += minBidIncrement;  // Increment the current bid value by minBidIncrement
                    lastBidder = autoBidUser; // Set the auto bidder as the last bidder
                    
                    canAutoBid = true;  // Continue auto-bidding
                } else if ((userBidAmount + minBidIncrement) > autoBidUserMaxAmount) {
                    // Mark that the user has reached their limit
                    String updateReachedLimit = "UPDATE bids SET Reached_Limit=1 WHERE username=? AND Clothing_ID=?";
                    pstmt = con.prepareStatement(updateReachedLimit);
                    pstmt.setString(1, autoBidUser);
                    pstmt.setInt(2, clothingID);
                    pstmt.executeUpdate();
                }
            }
        }
        
        // Update the final current bid value
        String updateCurrentBidValue = "UPDATE clothing SET Current_Bid_Value=? WHERE Clothing_ID=?";
        pstmt = con.prepareStatement(updateCurrentBidValue);
        pstmt.setFloat(1, userBidAmount);
        pstmt.setInt(2, clothingID);
        pstmt.executeUpdate();
        
        out.println("Bid process completed!");
        out.println("<br><a href='auctionDetails.jsp?clothingID=" + clothingID + "'>Back to Auction</a>");

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