import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.PreparedStatement;
import java.sql.SQLException;

public class AuctionProcessor {

    private static final String DATABASE_URL = "jdbc:mysql://localhost:3306/Clothes_to_my_Heart?useSSL=false";
    private static final String DATABASE_USER = "root";
    private static final String DATABASE_PASSWORD = "password";

    public static void processAuctions() {
        // Load JDBC driver (for older JDBC versions; might be optional for newer versions)
        try {
            Class.forName("com.mysql.jdbc.Driver");
        } catch (ClassNotFoundException e) {
            System.out.println("MySQL JDBC Driver not found. Ensure it's in the classpath.");
            e.printStackTrace();
            return;
        }

        try (Connection con = DriverManager.getConnection(DATABASE_URL, DATABASE_USER, DATABASE_PASSWORD)) {
            con.setAutoCommit(false);

            // Close expired auctions
            String closeExpiredAuctionsSQL = "UPDATE clothing SET isActive = 0 WHERE Closing_Date_Time < NOW() AND isActive = 1";
            try (PreparedStatement pstmt = con.prepareStatement(closeExpiredAuctionsSQL)) {
                pstmt.executeUpdate();
            }

            // Set the winner if the reserve price condition is met
            String setWinnerSQL = "UPDATE clothing c1 " + 
                    "JOIN bids b ON c1.Clothing_ID = b.Clothing_ID AND c1.Current_Bid_Value = b.Bid_Amount " + 
                    "SET c1.Winner_Username = b.username " +
                    "WHERE (c1.Reserve_Price IS NULL OR c1.Current_Bid_Value >= c1.Reserve_Price) " +
                    "AND c1.isActive = 0 AND c1.Winner_Username IS NULL";
            try (PreparedStatement pstmt = con.prepareStatement(setWinnerSQL)) {
                pstmt.executeUpdate();
            }

            // Update the earnings of the seller
            String updateEarningsSQL = "UPDATE account a INNER JOIN clothing c ON a.username = c.Seller_Username SET a.earnings = IFNULL(a.earnings, 0) + c.Current_Bid_Value WHERE c.isActive = 0 AND c.Winner_Username IS NOT NULL";
            try (PreparedStatement pstmt = con.prepareStatement(updateEarningsSQL)) {
                pstmt.executeUpdate();
            }

            con.commit();
        } catch (SQLException e) {
            System.out.println("SQL error occurred: " + e.getMessage());
            e.printStackTrace();
        }
    }

    public static void main(String[] args) {
        processAuctions();
    }
}