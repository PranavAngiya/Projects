<%@ page language="java" contentType="text/html; charset=UTF-8"
    pageEncoding="UTF-8"%>
<%@ page import="java.io.*,java.util.*,java.sql.*"%>
<%@ page import="javax.servlet.http.*,javax.servlet.*"%>
<!DOCTYPE html>
<html>
<head>
	<meta charset="UTF-8">
	<title>User Wish list</title>
	<link rel = "stylesheet" type="text/css" href="styles/user_home.css">
</head>
<body>
	<a href = "user_home.jsp"><button>Go Back</button></a>
	
	<h2>Create Wish List Item</h2>
	
	
	<form action="Create_Wish_List.jsp" method="post">
	
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
	
	<input type="submit" value="Submit">   
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
	
	
	
	

</body>
</html>