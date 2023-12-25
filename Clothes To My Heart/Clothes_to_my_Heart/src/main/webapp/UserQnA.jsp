<%@ page language="java" contentType="text/html; charset=UTF-8"
    pageEncoding="UTF-8"%>
    <%@ page import="java.io.*,java.util.*,java.sql.*"%>
<%@ page import="javax.servlet.http.*,javax.servlet.*"%>
<!DOCTYPE html>
<html>
<head>
	<meta charset="UTF-8">
	<title>Frequently Asked Questions</title>
	<link rel = "stylesheet" type="text/css" href="styles/welcome.css">
</head>

<body>
	<a href = "user_home.jsp"><button>Go Back</button></a>
	
	<br>
	<p>Post your own Question</p>
	<form method="post">
		<textarea name = "question" rows="4" cols="50"></textarea>
		<input type = "submit" name = "submit" value = "Submit">
	</form>
	
	<%
   	
   	if(request.getParameter("submit") != null){
	    String quser = (String) session.getAttribute("user");   
		String question = request.getParameter("question"); 
	    
	    if (question != null) {
		    Class.forName("com.mysql.jdbc.Driver");
		    Connection con = DriverManager.getConnection("jdbc:mysql://localhost:3306/Clothes_to_my_Heart","root", "password");
		    Statement st = con.createStatement();
		    PreparedStatement questionquery = con.prepareStatement("select * from qna where q_message = ?");
		    questionquery.setString(1, question);
		    ResultSet questionRS;
		    /* questionRS = st.executeQuery("select * from qna where q_message =" + question + ""); */
		    questionRS = questionquery.executeQuery();
		    if (questionRS.next()) {
		        out.println("<p>That exact question already exists</p>");
		    } 
		    else {
		    	PreparedStatement insertStatement = con.prepareStatement("INSERT INTO qna (q_username,q_message) VALUES (?, ?)");
		        insertStatement.setString(1, quser);
		        insertStatement.setString(2, question);
		    	int rowsAffected = insertStatement.executeUpdate();
		    	out.println("<p>Your question has been posted</p>");
		    }
	    }
   	}
	%>
	
	
	<br><br>
	
	<h2>Other Questions</h2>
	
	<p>Filer using Keyword</p>
	<form method="post">
		<textarea name = "filter" rows="1" cols="50"></textarea>
		<input type = "submit" name = "submit" value = "Submit">
	</form>
	
	
	<br>
	
	<%
		Class.forName("com.mysql.jdbc.Driver");
	    Connection con = DriverManager.getConnection("jdbc:mysql://localhost:3306/Clothes_to_my_Heart","root", "password");
	    Statement st = con.createStatement();
	    
	    if(request.getParameter("submit") != null){
	    	String filter = request.getParameter("filter");
	    	
	    	PreparedStatement filtersearch = con.prepareStatement("select * from qna where (q_message like ? or a_message like ?) and a_message is not null");
	    	filtersearch.setString(1,"%" + filter + "%");
	    	filtersearch.setString(2,"%" + filter + "%");
	    	ResultSet filterqnaRS = filtersearch.executeQuery();
	    	while(filterqnaRS.next()){
	    		String questionUsername = filterqnaRS.getString("q_username");
	    		String questionMessage = filterqnaRS.getString("q_message");
	    		String answerUsername = filterqnaRS.getString("a_username");
	    		String answerMessage = filterqnaRS.getString("a_message");
	    		
	    		%>
	    		
	    		<b>User: <%=questionUsername %></b>
	    		<p>Question: <%=questionMessage %></p>
	    		<p>Answered by Customer Representative: <%=answerUsername %></p>
	    		<p>Answer: <%=answerMessage %></p>
	    		<br><br>
	    		<%
		    }
	    	filtersearch.close();
	    	
	    	filtersearch = con.prepareStatement("select * from qna where q_message like ? and a_message is null");
	    	filtersearch.setString(1,"%" + filter + "%");
	    	filterqnaRS = filtersearch.executeQuery();
	    	while(filterqnaRS.next()){
	    		String questionUsername = filterqnaRS.getString("q_username");
	    		String questionMessage = filterqnaRS.getString("q_message");
	    		
	    		%>
	    		
	    		<b>User: <%=questionUsername %></b>
	    		<p>Question: <%=questionMessage %></p>
    			<p>Status: This question has not been answered yet</p>
	    		<br><br>
	    		<%
		    }
	    	filtersearch.close();
	    	
	    	
	    }
	    else{
		    ResultSet qnalistRS = st.executeQuery("select * from qna where a_message is not null");
		    
	    	while(qnalistRS.next()){
	    		String questionUsername = qnalistRS.getString("q_username");
	    		String questionMessage = qnalistRS.getString("q_message");
	    		String answerUsername = qnalistRS.getString("a_username");
	    		String answerMessage = qnalistRS.getString("a_message");
	    		
	    		%>
	    		
	    		<b>User: <%=questionUsername %></b>
	    		<p>Question: <%=questionMessage %></p>
	    		<p>Answered by Customer Representative: <%=answerUsername %></p>
	    		<p>Answer: <%=answerMessage %></p>
	    		<br><br>
	    		<%
		    }
	    	qnalistRS.close();
	    	
			qnalistRS = st.executeQuery("select * from qna where a_message is null");
		    
	    	while(qnalistRS.next()){
	    		String questionUsername = qnalistRS.getString("q_username");
	    		String questionMessage = qnalistRS.getString("q_message");
	    		
	    		%>
	    		
	    		<b>User: <%=questionUsername %></b>
	    		<p>Question: <%=questionMessage %></p>
	    		<p>Status: This question has not been answered yet</p>
	    		<br><br>
	    		<%
		    }
	    }
	    
	%>
	
</body>
</html>