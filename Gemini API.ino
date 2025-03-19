#include <WiFi.h>
#include <HTTPClient.h>
#include <WebServer.h>
#define LED_BUILTIN 2
const char* ssid = "ssid";
const char* password = "password";
const char* Gemini_Token = "Gemini_Token";
const char* Gemini_Max_Tokens = "500";

String res = ""; // To store the current question
String answer = ""; // To store the answer from the Gemini API
WebServer server(80); // WebServer object that will listen on port 80

//*IPAddress staticIP(192, 168, 137, 100);

//IPAddress gateway(192, 168, 137, 1);
//IPAddress subnet(255, 255, 255, 0);
//IPAddress dns(192, 168, 137, 1);
void setup() {
Serial.begin(115200);
WiFi.mode(WIFI_STA);
WiFi.disconnect();
pinMode(LED_BUILTIN, OUTPUT);
//if (WiFi.config(staticIP, gateway, subnet, dns, dns) == false) {
// Serial.println("Configuration failed.");
//}

// Wait for WiFi connection
WiFi.begin(ssid, password);
Serial.print("Connecting to ");
Serial.println(ssid);

while (WiFi.status() != WL_CONNECTED) {
delay(250);
Serial.print(".");
}
Serial.print("Local IP: ");
Serial.println(WiFi.localIP());
Serial.print("Subnet Mask: " );
Serial.println(WiFi.subnetMask());
Serial.print("Gateway IP: ");
Serial.println(WiFi.gatewayIP());
Serial.print("DNS 1: ");
Serial.println(WiFi.dnsIP(0));
Serial.print("DNS 2: ");
Serial.println(WiFi.dnsIP(1));
Serial.println("connected");
Serial.print("IP address: ");
Serial.println(WiFi.localIP());
delay(2000);

// Setup web server route
server.on("/", HTTP_GET, handleRoot); // Handle GET request for the root page
server.on("/ask", HTTP_POST, handleQuestion); // Handle POST request for submitting questions

// Start the server
server.begin();
}
void loop() {
if(WiFi.status() != WL_CONNECTED){WiFi.begin(ssid, password);
Serial.print("Connecting to ");
Serial.println(ssid);
while (WiFi.status() != WL_CONNECTED) {
delay(250);
Serial.print(".");
}
Serial.println(WiFi.localIP());delay(2000);}
server.handleClient(); // Handle incoming client requests
//if(x==0){Serial.println(WiFi.localIP());x=1;}
}

void handleRoot() {
String html = "

Ask a Question
";
// Display the form to input the question
html += "

";
html += "Enter your question:
";
// Use <textarea> for multi-line input
html += "<textarea id='question' name='question' rows='4' cols='50' required>" + res + "</textarea>

";
html += "";
html += "Clear"; // Clear button

// Add a div for the processing message
html += "

Processing... Please wait.
";
html += "

";
// Display the question and answer if available
//if (res.length() > 0 && answer.length() > 0) {
html += "

Your Question:
";
html += "
" + res + "

";
html += "
Answer:
";
html += "
" + answer + "

";
//}
// Add JavaScript to clear the input field and show the processing message
html += "<script>";
html += "function clearInput() {";
html += " document.getElementById('question').value = '';"; // Clear textarea value
html += "}";

// Function to show the 'Processing' message
html += "function showProcessingMessage() {";
html += " document.getElementById('loadingMessage').style.display = 'block';";
html += "}";
html += "</script>";

html += "";

// Send the HTML content with proper encoding
server.send(200, "text/html; charset=UTF-8", html); // Ensure UTF-8 charset in the response
}

void handleQuestion() {
// Get the question from the POST request
if (server.hasArg("question")) {
res = server.arg("question"); // Extract the question entered by the user
// Process the question and get the answer from the Gemini API
answer = getGeminiAnswer(res);

// Redirect back to the root page to display the answer
server.sendHeader("Location", "/");
server.send(303);  // HTTP status code for redirect
} else {
server.send(400, "text/html", "

Bad Request
");
}
if(answer=="Error: Could not extract text from response."){delay(1000); Serial.println("RETRY");handleQuestion();}
}
String getGeminiAnswer(String question) {
String response = "";

// Format the question as JSON
String formattedQuestion = """ + question + """;//+" answered should NEVER EVER have sub and super script and follow same type of language also don't give answer if question is for realtime based"

HTTPClient https;

if (https.begin("https://generativelanguage.googleapis.com/v1beta/models/gemini-1.5-flash:generateContent?key=" + (String)Gemini_Token)) {
https.addHeader("Content-Type", "application/json");

// Create the payload for the POST request
String payload = "{\"contents\": [{\"parts\":[{\"text\":" + formattedQuestion + "}]}],\"generationConfig\": {\"maxOutputTokens\": " + (String)Gemini_Max_Tokens + "}}";

int httpCode = https.POST(payload);
if (1) {
  String jsonResponse = https.getString();
  // Debugging: Print the full response
  Serial.println("Full Response: " + jsonResponse);
  
  // Extract the "text" field from the JSON response
  int textStart = jsonResponse.indexOf("text") + 8;
  int textEnd = jsonResponse.indexOf("finishReason", textStart)-53;
  
  if (textStart != -1 && textEnd != -1) {
    response = jsonResponse.substring(textStart, textEnd);
    // Filter the answer to handle newlines and unwanted characters
    String filteredAnswer = "";
    for (size_t i = 0; i < response.length()-12; i++) {
      char c = response[i];
      
      // Add the character to the filtered answer
      if(response[i]=='*' || response[i]=='`'){response[i]=' ';}
      //if(response[i]=='\\' && response[i+1]=='u' && response[i+5]=='c'){filteredAnswer += " < ";i=i+6;}
      //if(response[i]=='\\' && response[i+1]=='u' && response[i+5]=='e'){filteredAnswer += " > ";i=i+6;}
      if(response[i]=='\\' && response[i+1]=='n'){filteredAnswer += "<br>";i++;}
      else{filteredAnswer += response[i];}
    }
    response = filteredAnswer;
    Serial.println("replying");
    Serial.println(response);
  } else {
    response = "Error: Could not extract text from response.";
    return response;
  }
} else {
  response = "Error: HTTP request failed.";
}
https.end();
} else {
response = "Error: Unable to connect to API.";
}
return response;
}
