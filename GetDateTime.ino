// Open connection to the HTTP server
bool connect(const char* hostName, const int port) {
  bool ok = client.connect(hostName, port);
  return ok;
}
/* example  curl -vv http://www.google.com
* About to connect() to www.google.com port 80 (#0)
*   Trying 8.8.8.8...
* Connected to www.google.com (8.8.8.8) port 80 (#0)
> GET / HTTP/1.1
> User-Agent: curl/7.29.0
> Host: www.google.com
> Accept: * / *
*/
/* example response:
< HTTP/1.1 200 OK
< Date: Sun, 29 May 2016 10:00:14 GMT */
 
// Send the HTTP GET request to the server
bool sendRequest(const char* host, const char* resource) {
   client.print("GET ");
   client.print(resource);
   client.println(" HTTP/1.1");
   client.print("Host: ");
   client.println(host);
   client.println("Accept: */*");
   client.println("Connection: close");
   client.println();
   
   return true;
}
 
bool findDateAndTimeInResponseHeaders() {
   // Date and time string starts with Date: and ends with GMT
   // example: Date: Sun, 29 May 2016 10:00:14 GMT
   client.setTimeout(HTTP_TIMEOUT);
   char header[85];
   size_t length = client.readBytes(header, 85);
   header[length] = 0;
   
   String headerString = String(header);
   int ds = headerString.indexOf("Date: ");
   int de = headerString.indexOf("GMT");

   // Date and time: Sun, 29 May 2016 10:00:14
   dateAndTime = headerString.substring(ds+1, de);
   
   return dateAndTime.length()>15;
}

// Close the connection with the HTTP server
void disconnect() {
  client.stop();
}
 
//-------- time+date code ----------
// example: Sun, 29 May 2016 10:00:14
String extractDayNameFromDateTimeString(String dateTime) {
  uint8_t firstSpace = dateTime.indexOf(' ');
  String dayName = dateTime.substring(firstSpace+1, firstSpace+4);

  return dayName;
}

String extractDayFromDateTimeString(String dateTime) {
  uint8_t firstSpace = dateTime.indexOf(' ', 9);
  String dayStr = dateTime.substring(firstSpace+1, firstSpace+3);

  return dayStr;
}
 
String extractMonthFromDateTimeString(String dateTime) {
  uint8_t firstSpace = dateTime.indexOf(' ', 11);
  String monthStr = dateTime.substring(firstSpace+1, firstSpace+4);

  return monthStr;
}
 
String extractYearFromDateTimeString(String dateTime) {
  uint8_t firstSpace = dateTime.indexOf(' ', 14);
  String yearStr = dateTime.substring(firstSpace+1, firstSpace+5);

  return yearStr;
}
 
String extractHourFromDateTimeString(String dateTime) {
  uint8_t lastColon = dateTime.lastIndexOf(':');
  String hourStr = dateTime.substring(lastColon-3, lastColon-5);

  int h = hourStr.toInt();
  return String(h);
}
 
String extractMinuteFromDateTimeString(String dateTime) {
  uint8_t lastColon = dateTime.lastIndexOf(':');
  String minuteStr = dateTime.substring(lastColon, lastColon-2);

  return minuteStr;
}

String extractSecondFromDateTimeString(String dateTime) {
  uint8_t lastColon = dateTime.lastIndexOf(':');
  String secondStr = dateTime.substring(lastColon+1, lastColon+3);

  return secondStr;
}
 
String extractDayFromCalendarDate(String date) {
  String dateStr = String(date);
  uint8_t firstDot = dateStr.indexOf('.');
  String dayStr = dateStr.substring(1, firstDot);

  return dayStr;
}

String translateMonth(String monthStr) {
  if(monthStr.equals(String("Jan"))) return String("01");
  if(monthStr.equals(String("Feb"))) return String("02");
  if(monthStr.equals(String("Mar"))) return String("03");
  if(monthStr.equals(String("Apr"))) return String("04");
  if(monthStr.equals(String("May"))) return String("05");
  if(monthStr.equals(String("Jun"))) return String("06");
  if(monthStr.equals(String("Jul"))) return String("07");
  if(monthStr.equals(String("Aug"))) return String("08");
  if(monthStr.equals(String("Sep"))) return String("09");
  if(monthStr.equals(String("Oct"))) return String("10");
  if(monthStr.equals(String("Nov"))) return String("11");
  if(monthStr.equals(String("Dec"))) return String("12");
}

// Convert GMT to local;
String localDateAndTime() {
  String gmtDayName = extractDayNameFromDateTimeString(dateAndTime);
  int gmtDay = extractDayFromDateTimeString(dateAndTime).toInt();
  int gmtMonth = translateMonth(extractMonthFromDateTimeString(dateAndTime)).toInt();
  int gmtYear = extractYearFromDateTimeString(dateAndTime).toInt();
  int gmtHour = extractHourFromDateTimeString(dateAndTime).toInt();
  int gmtMinute = extractMinuteFromDateTimeString(dateAndTime).toInt();
  int gmtSecond = extractSecondFromDateTimeString(dateAndTime).toInt();
  
  // Leap year check
  if (gmtYear%4 == 0) daysAMonth[1] = 29; 

  // Time zone adjustment
  gmtHour += TIMEZONE;

  if (gmtHour < 0) {
    gmtHour += 24;
    gmtDay -= 1;
    
    if (gmtDay < 1) {
      if (gmtMonth == 1) {
        gmtMonth = 12;
        gmtYear -= 1;
      } else {
        gmtMonth -= 1;
      }

      if (gmtDayName == "Sun") {
      gmtDayName = "Saturday";
      }
      else if (gmtDayName == "Sat") {
        gmtDayName = "Friday";
      }
      else if (gmtDayName == "Fri") {
        gmtDayName = "Thursday";
      }
      else if (gmtDayName == "Thu") {
        gmtDayName = "Wednesday";
      }
      else if (gmtDayName == "Wed") {
        gmtDayName = "Tuesday";
      }
      else if (gmtDayName == "Tue") {
        gmtDayName = "Monday";
      }
      else if (gmtDayName == "Mon") {
        gmtDayName = "Sunday";
      }
      
      gmtDay = daysAMonth[gmtMonth-1];
    }
  }
  
  if (gmtHour >= 24) {
    gmtHour -= 24;
    gmtDay += 1;
    
    if (gmtDay > daysAMonth[gmtMonth-1]) {
      gmtDay = 1;
      gmtMonth += 1;
      
      if (gmtMonth > 12) {
        gmtYear += 1; 
      }
    }

    if (gmtDayName == "Sun") {
      gmtDayName = "Monday";
    }
    else if (gmtDayName == "Mon") {
      gmtDayName = "Tuesdsay";
    }
    else if (gmtDayName == "Tue") {
      gmtDayName = "Wednesday";
    }
    else if (gmtDayName == "Wed") {
      gmtDayName = "Thursday";
    }
    else if (gmtDayName == "Thu") {
      gmtDayName = "Friday";
    }
    else if (gmtDayName == "Fri") {
      gmtDayName = "Saturday";
    }
    else if (gmtDayName == "Sat") {
      gmtDayName = "Sunday";
    }
  }

  String localDateAndTime = gmtDayName;
  localDateAndTime += ", ";
  localDateAndTime += String(gmtDay);
  localDateAndTime += "-";
  localDateAndTime += String(gmtMonth);
  localDateAndTime += "-";
  localDateAndTime += String(gmtYear);
  localDateAndTime += " ";
  localDateAndTime += String(gmtHour);
  localDateAndTime += ":";
  localDateAndTime += String(gmtMinute);
  localDateAndTime += ":";
  localDateAndTime += String(gmtSecond);

  return localDateAndTime;
}
