 const char manager_html[] PROGMEM = R"rawliteral(
  <!DOCTYPE HTML>
  <html>
  <head>
      <title>Device Setup Manager</title>
      <meta charset="UTF-8">
      <meta name="viewport" content="width=device-width, initial-scale=1">
      <style>
        body {
          background-color: #f7f7f7;
          font-family: Trebuchet MS;
        }
        #submit {
          width: 150px;
          padding: 10px;
          cursor: pointer;
        }
        #spacer_20 {
          height: 20px;
        }
        #spacer_10{
        height: 10px;
        }
        fieldset {
          width:600px;
          background-color: #f7f7f7;
        }
        
      </style>
      <script>
        <!-- This function validates Restart Option -->
        function validateRestart() {
          if (!confirm("Are you sure you want to restart your device?")) {
          return false;
          }
      }
        
        <!-- This function checks if the MQTT text field is blank -->
        function validateNetworkFields() {
          const ipInput = document.getElementById('ipInput');
          const portInput = document.getElementById('portInput');
          const gatewayInput = document.getElementById('gatewayInput');
          const dnsInput = document.getElementById('dnsInput');
          const maskInput = document.getElementById('maskInput');
          if (ipInput.value === '' || !isValidIPAddress(ipInput.value)) {
            alert('The IP field must be filled in and in the format xxx.xxx.x.xxx');
            return false;
          }
          if(portInput.value === ''){
            alert('The Port field must be filled');
            return false;
          }
          if (gatewayInput.value === '' || !isValidIPAddress(gatewayInput.value)) {
            alert('The Gateway field must be filled in and in the format xxx.xxx.x.xxx');
            return false;
          }
          if (dnsInput.value === '' || !isValidIPAddress(dnsInput.value)) {
            alert('The DNS field must be filled in and in the format xxx.xxx.x.xxx');
            return false;
          }
          if (maskInput.value === '' || !isValidIPAddress(maskInput.value)) {
            alert('The Mask field must be filled in and in the format xxx.xxx.x.xxx');
            return false;
          }

          return true;
        }

        function isValidIPAddress(value) {
          const pattern = /^([0-9]{1,3}\.){3}[0-9]{1,3}$/;
          return pattern.test(value);
        }
      </script>
    </head>
    
  <body>
    <center>
    <fieldset class="my-fieldset">
      <h2>Network Setup</h2>
    </fieldset>
      <div id="spacer_10"></div>
      
      <fieldset class="my-fieldset">
          <legend><strong>Network Settings</strong></legend>
          <div id="spacer_20"></div>
            <form method="POST" action="/networksettings">
              <label for="ipInput">IP:</label>
              <input type="text" id="ipInput" name="ipInput" value="%IPADDRESS%" pattern="^([0-9]{1,3}\.){0,3}[0-9]{0,3}$" maxlength="15"><br><br>  
              <label for="portInput">Port:</label>
              <input type="text" id="portInput" name="portInput" value="%PORT%"><br><br> 	
              <label for="gatewayInput">Gateway:</label>
              <input type="text" id="gatewayInput" name="gatewayInput" value="%GATEWAY%" pattern="^([0-9]{1,3}\.){0,3}[0-9]{0,3}$" maxlength="15"><br><br>
              <label for="dnsInput">DNS:</label>
              <input type="text" id="dnsInput" name="dnsInput" value="%DNS%" pattern="^([0-9]{1,3}\.){0,3}[0-9]{0,3}$" maxlength="15"><br><br> 
              <label for="maskInput">Mask:</label>
              <input type="text" id="maskInput" name="maskInput" value="%MASK%" pattern="^([0-9]{1,3}\.){0,3}[0-9]{0,3}$" maxlength="15"><br><br>
              <input type="submit" id="submit" value="Save" onclick="return validateNetworkFields()"> 
            </form> 
            <div id="spacer_20"></div>
        </fieldset>
        
        <div id="spacer_20"></div>
      <fieldset class="my-fieldset">
        <legend><strong>Informations</strong></legend>
        <div id="spacer_20"></div>
          <form method="GET" action="/restartdevice">
            <label for="macAddress">MAC Adress: %MACADDRESS%</label>
            <span id="macAddress"></span><br><br>
            <label for="firmwareVersion">Firmware Version: %FIRMWAREVERSION%</label>
            <span id="firmwareVersion"></span><br><br>
            <div id="spacer_20"></div>
            <input type="submit" id="submit" value="Restart Device" onclick="return validateRestart()">
              <div id="spacer_20"></div>
        </form> 
      </fieldset>     
    </center>
  </body>
  </html>)rawliteral";
