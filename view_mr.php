<?
$connect = mysqli_connect("localhost", "root", "raspberry", "rpidb");

if (mysqli_connect_errno()){
 echo "MySQL 연결에 실패하였습니다 : " . mysqli_connect_error();
}

$gas = '';
$flame = '';
$illuminance = '';
$motion = '';
$time = '';
 
$result = mysqli_query($connect,"SELECT * FROM envdata");

while($row = mysqli_fetch_array($result)) {
    $gas = $row['gas'];
    $flame = $row['flame'];
    $illuminance = $row['illuminance'];
    $motion = $row['motion'];
    $time = $row['time'];
}

mysqli_close($connect);
?>

<html>
<head>
	<meta http-equiv="Content-Type" content="text/html" charset="utf-8">
	<script language="JavaScript">
		setTimeout("history.go(0)", 1000);
	</script>
	
    <style type="text/css">
	.tblGreen {    border-collapse: collapse;    text-align: center;    font-family: 'Trebuchet MS';}.tblGreen td, th {    font-size: 10pt;    border: 1px solid #98bf21;    height: 30px;}.tblGreen th {    background-color:#A7C942;    color:#ffffff;    font-family: Georgia;}.tblGreen tr.alt td {    color:#000000;    background-color:#EAF2D3;}.tblGreen caption {    height: 30px;    text-align: left;    font-weight: bold;}
    </style>
 
	<body>
	    <table class="tblGreen">    
	    	<caption>센서 정보</caption>   
	    	 <colgroup>               
	    	 	 <col width="150">       
	    	 	  <col width="100">
	    	 	  <col width="100">
	    	 	  <col width="100">
	    	 	  <col width="100">
	    	  </colgroup>   
	    	   
	    	   <thead>        
		    	   	<tr>            
		    	   		<th>시간</th>            
		    	   		<th>조도값</th>
		    	    </tr>    
		       </thead>    
		       
		       <tbody>       
		           <tr>                        
		               <td><?= $time ?></td>            
		               <td><?= $illuminance ?></td>
		           </tr>
		       </tbody>    
		   </table>
	</body>

</head>
</html>