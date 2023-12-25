<%@ page language="java" contentType="text/html; charset=UTF-8"
    pageEncoding="UTF-8"%>
<%@ page import="java.io.*,java.util.*,java.sql.*"%>
<%@ page import="javax.servlet.http.*,javax.servlet.*"%>
<!DOCTYPE html>
<html>
<head>
	<meta charset="UTF-8">
	<title>Answer Questions</title>
	<link rel = "stylesheet" type="text/css" href="styles/welcome.css">
</head>

<body>
	<a href = "rep_home.jsp"><button>Go Back</button></a>
	<h2>Questions from users</h2>
	<br>
	
	<%
		Class.forName("com.mysql.jdbc.Driver");
	    Connection con = DriverManager.getConnection("jdbc:mysql://localhost:3306/Clothes_to_my_Heart","root", "password");
	    Statement st = con.createStatement();
	    
		ResultSet qnalistRS = st.executeQuery("select * from qna where a_message is null");
	    
    	while(qnalistRS.next()){
    		int qnaID = qnalistRS.getInt("qnaid");
    		String questionUsername = qnalistRS.getString("q_username");
    		String questionMessage = qnalistRS.getString("q_message");
    		%>
    		
    		<b>User: <%=questionUsername %></b>
    		<p>Question: <%=questionMessage %></p>
    		<p>Post Answer</p>
    		<form method="post">
				<textarea name = "answer" rows="4" cols="50"></textarea>
	    		<input type="hidden" name="qnaID" value="<%= qnaID %>"/>
				<input type = "submit" name = "submit" value = "Submit">
			</form>
			
			<%
		   	if(request.getParameter("submit") != null){

		   		String questionandanswerID = request.getParameter("qnaID");
			    String auser = (String) session.getAttribute("user");   
				String answer = request.getParameter("answer"); 
			    
			    if (answer != null) {
			    	PreparedStatement insertStatement = con.prepareStatement("update qna set a_username = ? , a_message = ? where qnaid = ?");
			        insertStatement.setString(1, auser);
			        insertStatement.setString(2, answer);
			        insertStatement.setString(3, questionandanswerID);
			    	int rowsAffected = insertStatement.executeUpdate();
			    	out.println("<p>Your Answer</p>");
			    }
		   	}
			%>
			
			<p>Delete Question?</p>
			<form method="post">
	    		Type Yes to Delete Question: <input type = "text" name = "confirm"/> <br/>
	    		<input type="hidden" name="qnaID" value="<%= qnaID %>"/>
	    		<input type="submit" name = "submit" value="Submit"/>
	    	</form>
	    	
	    	<%
   	
		   	if(request.getParameter("submit") != null){
		   		String questionandanswerID = request.getParameter("qnaID");
				String confirmation = request.getParameter("confirm"); 
			    
			    if (confirmation != null) {
			    	if(confirmation.equals("Yes")){
			    		PreparedStatement insertStatement = con.prepareStatement("delete from qna where qnaid = ?");
				        insertStatement.setString(1, questionandanswerID);
				    	int rowsAffected = insertStatement.executeUpdate();
				    	out.println("<p>Deleted Post</p>");
			    	}
			    	else{
			    		out.println("Please type Yes to confirm");
			    	}
			    	
				    
			    }
		   	}
			%>
			
			<br><br><br>
    		
    		<%
	    }
    	
    	qnalistRS.close();
    	
    	
		qnalistRS = st.executeQuery("select * from qna where a_message is not null");
	    
    	while(qnalistRS.next()){
    		int qnaID = qnalistRS.getInt("qnaid");
    		String questionUsername = qnalistRS.getString("q_username");
    		String questionMessage = qnalistRS.getString("q_message");
    		String answerUsername = qnalistRS.getString("a_username");
    		String answerMessage = qnalistRS.getString("a_message");
    		
    		%>
    		
    		<b>User: <%=questionUsername %></b>
    		<p>Question: <%=questionMessage %></p>
    		<p>Answered by Customer Representative: <%=answerUsername %></p>
    		<p>Answer: <%=answerMessage %></p>
    		<p>Delete Question?</p>
			<form method="post">
	    		Type Yes to Delete Question: <input type = "text" name = "confirm"/> <br/>
	    		<input type="hidden" name="qnaID" value="<%= qnaID %>"/>
	    		<input type="submit" name = "submit" value="Submit"/>
	    	</form>
	    	
	    	<%
   	
		   	if(request.getParameter("submit") != null){
		   		String questionandanswerID = request.getParameter("qnaID");
				String confirmation = request.getParameter("confirm"); 
			    
			    if (confirmation != null ) {
			    	if(confirmation.equals("Yes")){
			    		PreparedStatement insertStatement = con.prepareStatement("delete from qna where qnaid = ?");
				        insertStatement.setString(1, questionandanswerID);
				    	int rowsAffected = insertStatement.executeUpdate();
				    	
				    	%>
				    	
				    	<p>Deleted Post</p>
				    	
				    	<% 
			    	}
			    	else{
			    		out.println("Please type Yes to confirm");
			    	}
			    	
				    
			    }
		   	}
			%>
    		<br><br>
    		<%
	    }
	    
	%>
	
	

</body>
</html>