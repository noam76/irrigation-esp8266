static const char PROGMEM INDEX_HTML[] = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta charset="utf-8" />
 <title>Irrigation Viewer</title>
 <link href='https://fonts.googleapis.com/css?family=Rock+Salt' rel='stylesheet' type='text/css'>
 <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/4.7.0/css/font-awesome.min.css">
 <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.4.0/css/fontawesome.min.css" integrity="sha512-SgaqKKxJDQ/tAUAAXzvxZz33rmn7leYDYfBP+YoMRSENhf3zJyx3SBASt/OfeQwBHA1nxMis7mM3EV/oYT6Fdw==" crossorigin="anonymous" referrerpolicy="no-referrer" />
 <!-- <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/5.15.3/css/all.min.css"> -->
<script type="text/javascript" src="https://ajax.googleapis.com/ajax/libs/jquery/3.6.0/jquery.min.js"></script>

<style type="text/css">
/**
 * Layout
 */
body {
  background-color: #ccc;
  padding: 0% 3% 0% 3%;
  font-size: 14px;
  font-family: Lato, sans-serif;
}

.main {
  width: 80%;
  display: flex;
  flex-direction: column;
  margin-top: 75px;
}

.wrapper {
  display: flex;
  justify-content: space-between;
}

.sidebar {
  width: 15%;
  height: 25vh;
  min-height: 150px;
  position: -webkit-sticky;
  position: sticky;
  top: 20%;
  text-align: center;
  border: 3px solid purple;
  padding: 17px;
  margin-left: 5px;
}

ins {
  text-decoration: none;
}

h2 {
  margin: 0;
  padding: 0;
}

table {
  table-layout: fixed;
  width: 103%;
  border-collapse: collapse;
  border: 3px solid purple;
}

thead th, tfoot th {
  font-family: 'Rock Salt', cursive;
}

th {
  letter-spacing: 2px;
}

td {
  letter-spacing: 1px;
}

tbody td {
  text-align: center;
}

thead {
  color: white;
  text-shadow: 1px 1px 1px black;
}

tfoot {
  color: black;
  text-align: center;
}

thead th, tfoot th, tfoot td {
  background: linear-gradient(to bottom, rgba(0,0,0,0.1), rgba(0,0,0,0.5));
  border: 3px solid purple;
}

th, td {
  padding: 10px;
}

tbody tr:nth-child(odd) {
  background-color: #ffffff;
}

tbody tr:nth-child(even) {
  background-color: #EB984E;
}

/**
 * Sticky navigation
 */
.sticky {
  text-align: center;
  background-color: #ccc;
  position: -webkit-sticky;
  position: fixed;
  top: 0;
  height: 63px;
  width: 100%;
}

/**
 * Display lists on a single line.
 * @bugfix Prevent webkit from removing list semantics
 * 1. Add a non-breaking space
 * 2. Make sure it doesn't mess up the DOM flow
 */
.list-inline {
  list-style: none;
  margin-left: -0.5em;
  margin-right: -0.5em;
  padding: 0;
}

.list-inline > li {
  display: inline-block;
  margin-left: 0.5em;
  margin-right: 0.5em;
}

.list-inline > li:before {
  content: "\200B"; /* 1 */
  position: absolute; /* 2 */
}

input[type=submit]{
  background-color: #EB984E;
  border-radius: 10px;
  border: 0;
  width: 80px;
  height:30px;
  text-decoration: none;
  cursor: pointer;
}

.sidebar-send-btn {
  background-color: #EB984E;
  border-radius: 10px;
  border: 0;
  width: 80px;
  height: 30px;
  text-decoration: none;
  cursor: pointer;
}
</style>
</head>
 <body>
 <div class="sticky">
   <h1>Irrigation System Viewer</h1>
 </div>
 <div class="wrapper">
   <div class="sidebar">
   <h3>Pump State</h3>
   <div id="pump_status"></div>
   <br>
   <div><i class="fa fa-refresh fa-2x fa-fw"></i></div>
  <br><br><br><br><br><br><br><br><br>
  <table>
    <tr>
      <th scope="row"><i class="fa fa-paper-plane fa-2x" aria-hidden="true"></i> Send Data</th>
      <td><button class="sidebar-send-btn" onclick="document.getElementById('myForm').submit()">Send</button></td>
    </tr>
  </table>
  </div> <!--  end sidebar -->
 <div class="main">
 <form id="myForm" action="/get" method="POST">
<table>
   <thead>
     <tr>
       <th scope="col">Plant</th>
       <th scope="col"><i class="fa fa-thermometer-three-quarters fa-2x" aria-hidden="true"></i> Humidity %</th>
       <th scope="col">Tap Status</th>
       <th scope="col">Tap Icon</th>
     <th scope="col" colspan="2">Select humidity rate</th>
     </tr>
   </thead>
   <tbody>
     <tr>
       <th scope="row"><i class="fa fa-tree fa-2x" aria-hidden="true"></i> Sensor 0</th>
       <td id="sensor0"></td>
       <td id="solenoid-status0"></td>
       <td><i class="fa fa-refresh fa-2x fa-fw"></i></td>
     <td>Max: <ins id="userMax0"></ins> <input type="text" name="userMax[0]" maxlength="4" size="4" oninput="checkInput(this);"></td>
     <td>Min: <ins id="userMin0"></ins> <input type="text" name="userMin[0]" maxlength="4" size="4" oninput="checkInput(this);"></td>
     </tr>
     <tr>
       <th scope="row"><i class="fa fa-tree fa-2x" aria-hidden="true"></i> Sensor 1</th>
       <td id="sensor1"></td>
       <td id="solenoid-status1"></td>
       <td><i class="fa fa-refresh fa-2x fa-fw"></i></td>
     <td>Max: <ins id="userMax1"></ins> <input type="text" name="userMax[1]" maxlength="4" size="4" oninput="checkInput(this);"></td>
     <td>Min: <ins id="userMin1"></ins> <input type="text" name="userMin[1]" maxlength="4" size="4" oninput="checkInput(this);"></td>
     </tr>
 
     <tr>
       <th scope="row"><i class="fa fa-tree fa-2x" aria-hidden="true"></i> Sensor 2</th>
       <td id="sensor2"></td>
       <td id="solenoid-status2"></td>
       <td><i class="fa fa-refresh fa-2x fa-fw"></i></td>
     <td>Max: <ins id="userMax2"></ins> <input type="text" name="userMax[2]" maxlength="4" size="4" oninput="checkInput(this);"></td>
     <td>Min: <ins id="userMin2"></ins> <input type="text" name="userMin[2]" maxlength="4" size="4" oninput="checkInput(this);"></td>
     </tr>
 
     <tr>
       <th scope="row"><i class="fa fa-tree fa-2x" aria-hidden="true"></i> Sensor 3</th>
       <td id="sensor3"></td>
       <td id="solenoid-status3"></td>
       <td><i class="fa fa-refresh fa-2x fa-fw"></i></td>
     <td>Max: <ins id="max03"></ins> <input type="text" name="userMax[3]" maxlength="4" size="4" oninput="checkInput(this);"></td>
     <td>Min: <ins id="min03"></ins> <input type="text" name="userMin[3]" maxlength="4" size="4" oninput="checkInput(this);"></td>
     </tr>
  
  <tr>
       <th scope="row"><i class="fa fa-tree fa-2x" aria-hidden="true"></i> Sensor 4</th>
       <td id="sensor4"></td>
       <td id="solenoid-status4"></td>
       <td><i class="fa fa-refresh fa-2x fa-fw"></i></td>
     <td>Max: <ins id="max04"></ins> <input type="text" name="userMax[4]" maxlength="4" size="4" oninput="checkInput(this);"></td>
     <td>Min: <ins id="min04"></ins> <input type="text" name="userMin[4]" maxlength="4" size="4" oninput="checkInput(this);"></td>
     </tr>
  
  <tr>
       <th scope="row"><i class="fa fa-tree fa-2x" aria-hidden="true"></i> Sensor 5</th>
       <td id="sensor5"></td>
       <td id="solenoid-status5"></td>
       <td><i class="fa fa-refresh fa-2x fa-fw"></i></td>
     <td>Max: <ins id="max05"></ins> <input type="text" name="userMax[5]" maxlength="4" size="4" oninput="checkInput(this);"></td>
     <td>Min: <ins id="min05"></ins> <input type="text" name="userMin[5]" maxlength="4" size="4" oninput="checkInput(this);"></td>
     </tr>
  
  <tr>
       <th scope="row"><i class="fa fa-tree fa-2x" aria-hidden="true"></i> Sensor 6</th>
       <td id="sensor6"></td>
       <td id="solenoid-status6"></td>
       <td><i class="fa fa-refresh fa-2x fa-fw"></i></td>
     <td>Max: <ins id="max06"></ins> <input type="text" name="userMax[6]" maxlength="4" size="4" oninput="checkInput(this);"></td>
     <td>Min: <ins id="min06"></ins> <input type="text" name="userMin[6]" maxlength="4" size="4" oninput="checkInput(this);"></td>
     </tr>
  
  <tr>
       <th scope="row"><i class="fa fa-tree fa-2x" aria-hidden="true"></i> Sensor 7</th>
       <td id="sensor7"></td>
       <td id="solenoid-status7"></td>
       <td><i class="fa fa-refresh fa-2x fa-fw"></i></td>
     <td>Max: <ins id="max07"></ins> <input type="text" name="userMax[7]" maxlength="4" size="4" oninput="checkInput(this);"></td>
     <td>Min: <ins id="min07"></ins> <input type="text" name="userMin[7]" maxlength="4" size="4" oninput="checkInput(this);"></td>
     </tr>
  
  <tr>
       <th scope="row"><i class="fa fa-tree fa-2x" aria-hidden="true"></i> Sensor 8</th>
       <td id="sensor8"></td>
       <td id="solenoid-status8"></td>
       <td><i class="fa fa-refresh fa-2x fa-fw"></i></td>
     <td>Max: <ins id="max08"></ins> <input type="text" name="userMax[8]" maxlength="4" size="4" oninput="checkInput(this);"></td>
     <td>Min: <ins id="min08"></ins> <input type="text" name="userMin[8]" maxlength="4" size="4" oninput="checkInput(this);"></td>
     </tr>
  
  <tr>
       <th scope="row"><i class="fa fa-tree fa-2x" aria-hidden="true"></i> Sensor 9</th>
       <td id="sensor9"></td>
       <td id="solenoid-status9"></td>v
       <td><i class="fa fa-refresh fa-2x fa-fw"></i></td>
     <td>Max: <ins id="max09"></ins> <input type="text" name="userMax[9]" maxlength="4" size="4" oninput="checkInput(this);"></td>
     <td>Min: <ins id="min09"></ins> <input type="text" name="userMin[9]" maxlength="4" size="4" oninput="checkInput(this);"></td>
     </tr>
  
  <tr>
       <th scope="row"><i class="fa fa-tree fa-2x" aria-hidden="true"></i> Sensor 10</th>
       <td id="sensor10"></td>
       <td id="solenoid-status10"></td>
       <td><i class="fa fa-refresh fa-2x fa-fw"></i></td>
     <td>Max: <ins id="max10"></ins> <input type="text" name="userMax[10]" maxlength="4" size="4" oninput="checkInput(this);"></td>
     <td>Min: <ins id="min10"></ins> <input type="text" name="userMin[10]" maxlength="4" size="4" oninput="checkInput(this);"></td>
     </tr>
  
  <tr>
       <th scope="row"><i class="fa fa-tree fa-2x" aria-hidden="true"></i> Sensor 11</th>
       <td id="sensor11"></td>
       <td id="solenoid-status11"></td>
       <td><i class="fa fa-refresh fa-2x fa-fw"></i></td>
     <td>Max: <ins id="max11"></ins> <input type="text" name="userMax[11]" maxlength="4" size="4" oninput="checkInput(this);"></td>
     <td>Min: <ins id="min11"></ins> <input type="text" name="userMin[11]" maxlength="4" size="4" oninput="checkInput(this);"></td>
     </tr>
  
  <tr>
       <th scope="row"><i class="fa fa-tree fa-2x" aria-hidden="true"></i> Sensor 12</th>
       <td id="sensor12"></td>
       <td id="solenoid-status12"></td>
       <td><i class="fa fa-refresh fa-2x fa-fw"></i></td>
     <td>Max: <ins id="max12"></ins> <input type="text" name="userMax[12]" maxlength="4" size="4" oninput="checkInput(this);"></td>
     <td>Min: <ins id="min12"></ins> <input type="text" name="userMin[12]" maxlength="4" size="4" oninput="checkInput(this);"></td>
     </tr>
  
  <tr>
       <th scope="row"><i class="fa fa-tree fa-2x" aria-hidden="true"></i> Sensor 13</th>
       <td id="sensor13"></td>
       <td id="solenoid-status13"></td>
       <td><i class="fa fa-refresh fa-2x fa-fw"></i></td>
     <td>Max: <ins id="max13"></ins> <input type="text" name="userMax[13]" maxlength="4" size="4" oninput="checkInput(this);"></td>
     <td>Min: <ins id="min13"></ins> <input type="text" name="userMin[13]" maxlength="4" size="4" oninput="checkInput(this);"></td>
     </tr>
  
  <tr>
       <th scope="row"><i class="fa fa-tree fa-2x" aria-hidden="true"></i> Sensor 14</th>
       <td id="sensor14"></td>
       <td id="solenoid-status14"></td>
       <td><i class="fa fa-refresh fa-2x fa-fw"></i></td>
     <td>Max: <ins id="max14"></ins> <input type="text" name="userMax[14]" maxlength="4" size="4" oninput="checkInput(this);"></td>
     <td>Min: <ins id="min14"></ins> <input type="text" name="userMin[14]" maxlength="4" size="4" oninput="checkInput(this);"></td>
     </tr>
  
  <tr>
       <th scope="row"><i class="fa fa-tree fa-2x" aria-hidden="true"></i> Sensor 15</th>
       <td id="sensor15"></td>
       <td id="solenoid-status15"></td>
       <td><i class="fa fa-refresh fa-2x fa-fw"></i></td>
     <td>Max: <ins id="max15"></ins> <input type="text" name="userMax[15]" maxlength="4" size="4" oninput="checkInput(this);"></td>
     <td>Min: <ins id="min15"></ins> <input type="text" name="userMin[15]" maxlength="4" size="4" oninput="checkInput(this);"></td>
     </tr>
   </tbody>
  </table>
  </form>
  <br>
  </div> <!-- end main -->
 </div> <!-- end wrapper -->
<script type="text/javascript">
 function checkInput(input) {
  if (input.value !== '' && !/^\d{1,3}$/.test(input.value)) {
    alert("La valeur doit être un nombre de 1 à 3 chiffres.");
    input.value = "";
  }
 }
 
 function sendForm() {
  document.getElementById("myForm").submit();
 }
 
 document.getElementById("myForm").addEventListener("submit", function(event) {
  event.preventDefault(); // prevent the form from submitting
  // displayFormValues(); // call the function to display the form values
 });

$(document).ready(function() {
  setInterval(function() {
    $.ajax({
      url: "/sendata",
      dataType: "xml",
      success: function(xml) {
        // Display sensor values
        for (var i = 0; i < 15; i++) {
          var sensor_value = $(xml).find("mux sensor[id='" + i + "']").text();
          sensor_value += " %"; // Ajouter le symbole %
          //console.log("Sensor " + i + " value: " + sensor_value);
          $("#sensor" + i).text(sensor_value);
        }
        
        // Display solenoid status
        for (var i = 0; i < 15; i++) {
          var solenoid_status = $(xml).find("mux solenoid[id='" + i + "']").text();
          var status_color = (solenoid_status === "0") ? "#ff0000" : "greenyellow";
          var status_text = (solenoid_status === "0") ? "OFF" : "ON";
          $("#solenoid-status" + i).text(status_text).css({color: status_color, fontWeight: "bold"});
        }
		
		// Display Usemax Usermin status
		for (var i = 0; i < 15; i++) {
          var userMax_value = $(xml).find("mux userMax[id='" + i + "']").text();
		  var userMin_value = $(xml).find("mux userMin[id='" + i + "']").text();
          console.log("userMax " + i + " value: " + userMax_value);
		  console.log("userMin " + i + " value: " + userMin_value);
          $("#userMax" + i).text(userMax_value);
		  $("#userMin" + i).text(userMin_value);
        }

        // Display pump status
        var pump_status = $(xml).find("pump").text();
        //console.log("Pump status: " + pump_status);
        if (pump_status === "1") {
          $("#pump_status").text("On").css({color: "green", fontWeight: "bold"});
        } else {
          $("#pump_status").text("Off").css({color: "red", fontWeight: "bold"});
        }
    
        // Remove existing rows for sensors
        $("tr.sensor-row").remove();
    
        // Add rows for each sensor in sensorValuesTotal
        for (var i = 0; i < 15; i++) {
          // var sensor_value = sensorValuesTotal[i];
          var sensor_value = $(this).attr("id");
          var row = "<tr class='sensor-row'><td>Sensor " + i + "</td><td id='sensor" + i + "'>" + sensor_value + "</td><td id='solenoid-status" + i + "'></td><td><i class='fa fa-refresh fa-2x fa-fw'></i></td></tr>";
          $("#sensor-table" + i).append(row);
        }
      },
      error: function(xhr, status, error) {
        console.log("Error: " + error.message);
      }
    });
  }, 1000);
});

 </script>
</body>
</html>
)rawliteral";
