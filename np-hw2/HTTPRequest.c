//
//  HTTPRequest.c
//  Proxy Server
//
//  Created by Matt Zanchelli on 3/1/14.
//  Copyright (c) 2014 Matt Zanchelli. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "HTTPRequest.h"

HTTPRequest HTTPRequestCreate()
{
	// Create an array of strings; one for each header field.
	return (HTTPRequest) calloc(HTTPRequestHeaderFieldsCount, sizeof(char *));
}

void HTTPRequestFree(HTTPRequest request)
{
	// Free all the strings contained in the request.
	for ( int i=0; i<HTTPRequestHeaderFieldsCount; ++i ) {
		// Only if there is a value for this field.
		if ( request[i] != NULL ) {
			free(request[i]);
			request[i] = NULL;
		}
	}
	
	// Free the request itself.
	free(request);
}

bool validateRequest(HTTPRequest request)
{
	/*
	 Request-Line specifications via http://www.w3.org/Protocols/rfc2616/rfc2616-sec5.html
	 Request-Line = Method SP Request-URI SP HTTP-Version CRLF
	 */
	
	char *Request_Line = request[HTTPRequestHeaderField_Request_Line];
	
	char *Method = NULL;
	char *MethodArgs = NULL;
	if ( !splitStringAtString(Request_Line, " ", &Method, &MethodArgs) ) {
		// Invalid Request-Line.
		return false;
	}
	
	char *Request_URI = NULL;
	char *HTTP_Version = NULL;
	if ( !splitStringAtString(MethodArgs, " ", &Request_URI, &HTTP_Version) ) {
		// Invalid Request-Line.
		return false;
	}
	
	// TODO: Check if valid Request-URI.
	
	// Check if supported HTTP-Version.
	if ( stringEquality(HTTP_Version, "HTTP/1.1") ) {
		// HTTP/1.1 is supported.
	} else if ( stringEquality(HTTP_Version, "HTTP/1.0") ) {
		// HTTP/1.0 is supported.
	} else {
		// Unsupported or invalid HTTP-Version.
		return false;
	}
	
	// Must include Host.
	if ( request[HTTPRequestHeaderField_Host] == NULL ) {
		return false;
	}
	
	// If POST, must include Content-Length.
	if ( stringEquality(Method, "POST") ) {
		if ( request[HTTPRequestHeaderField_Content_Length] == NULL ) {
			return false;
		}
	}
	
	// No errors are found.
	return true;
}

char *requestStringFromRequest(HTTPRequest request)
{
	char *requestLine = request[HTTPRequestHeaderField_Request_Line];
	if ( !requestLine ) {
		return NULL;
	}
	
	// Modify request string to remove host from request uri.
	char *modifiedRequestLine = stringDuplicate(requestLine);
	char *URI_Start = strchr(modifiedRequestLine, (int) ' ') + 1;
	char *URI_Base_End = requestLine;
	URI_Base_End = strchr(URI_Base_End, (int) '/') + 1;
	URI_Base_End = strchr(URI_Base_End, (int) '/') + 1;
	URI_Base_End = strchr(URI_Base_End, (int) '/');
	if ( URI_Base_End == NULL ) {
		// Find the second space and then add '/' manually.
		char *secondSpace = strchr(modifiedRequestLine, (int) ' ');
		strcpy(URI_Start, "/");
		URI_Base_End = secondSpace;
	}
	strcpy(URI_Start, URI_Base_End);
	free(requestLine);
	requestLine = modifiedRequestLine;
	request[HTTPRequestHeaderField_Request_Line] = requestLine;
	
	// Start with the requestLine.
	char *string = stringDuplicate(requestLine);
	
	for ( int field=0; field<HTTPRequestHeaderFieldsCount; ++field ) {
		// Skip the request line.
		if ( field == HTTPRequestHeaderField_Request_Line ) {
			continue;
		}
		
		// Get the value of the field.
		char *fieldValue = request[field];
		if ( fieldValue == NULL || strlen(fieldValue) < 1 ) {
			// The field does not have a value, skip.
			continue;
		}
		
		// Get the name of the field.
		const char *fieldName = HTTPRequestHeaderFieldNameForField(field);
		
		// Find length of a new string.
		unsigned long newLength = strlen(string) + strlen("\r\n") + strlen(fieldName) + strlen(": ") + strlen(fieldValue) + 1;
		// Allocate new string.
		char *newString = malloc(sizeof(char) * newLength);
		// Copy over original string.
		strcpy(newString, string);
		// Original string is no longer needed.
		free(string);
		// Add a carriage return and a newline.
		strcat(newString, "\r\n");
		// Add the field name.
		strcat(newString, fieldName);
		// Add a colon and a space separator.
		strcat(newString, ": ");
		// Add the field value.
		strcat(newString, fieldValue);
		
		// Point string to the new string.
		string = newString;
	}
	
	// End the request.
	// Find the required length of the buffer.
	unsigned long newLength = strlen(string) + strlen("\r\n\r\n") + 1;
	// Allocate the new string.
	char *newString = malloc(sizeof(char) * newLength);
	// Copy the string over.
	strcpy(newString, string);
	// Add the carriage returns and newlines.
	strcat(newString, "\r\n\r\n");
	// The old string is no longer necessary.
	free(string);
	// Point string to the new one.
	string = newString;
	
	// Return the full request string.
	return string;
}

HTTPRequestHeaderField HTTPRequestHeaderFieldForFieldNamed(HTTPRequestHeaderFieldName fieldName)
{
	if ( stringEquality(fieldName, HTTPRequestHeaderFieldName_Accept) ) {
		return HTTPRequestHeaderField_Accept;
	} else if ( stringEquality(fieldName, HTTPRequestHeaderFieldName_Accept_Charset) ) {
		return HTTPRequestHeaderField_Accept_Charset;
	} else if ( stringEquality(fieldName, HTTPRequestHeaderFieldName_Accept_Encoding) ) {
		return HTTPRequestHeaderField_Accept_Encoding;
	} else if ( stringEquality(fieldName, HTTPRequestHeaderFieldName_Accept_Language) ) {
		return HTTPRequestHeaderField_Accept_Language;
	} else if ( stringEquality(fieldName, HTTPRequestHeaderFieldName_Accept_Datetime) ) {
		return HTTPRequestHeaderField_Accept_Datetime;
	} else if ( stringEquality(fieldName, HTTPRequestHeaderFieldName_Authorization) ) {
		return HTTPRequestHeaderField_Authorization;
	} else if ( stringEquality(fieldName, HTTPRequestHeaderFieldName_Cache_Control) ) {
		return HTTPRequestHeaderField_Cache_Control;
	} else if ( stringEquality(fieldName, HTTPRequestHeaderFieldName_Connection) ) {
		return HTTPRequestHeaderField_Connection;
	} else if ( stringEquality(fieldName, HTTPRequestHeaderFieldName_Proxy_Connection) ) {
		return HTTPRequestHeaderField_Proxy_Connection;
	} else if ( stringEquality(fieldName, HTTPRequestHeaderFieldName_Cookie) ) {
		return HTTPRequestHeaderField_Cookie;
	} else if ( stringEquality(fieldName, HTTPRequestHeaderFieldName_Content_Length) ) {
		return HTTPRequestHeaderField_Content_Length;
	} else if ( stringEquality(fieldName, HTTPRequestHeaderFieldName_Content_MD5) ) {
		return HTTPRequestHeaderField_Content_MD5;
	} else if ( stringEquality(fieldName, HTTPRequestHeaderFieldName_Content_Type) ) {
		return HTTPRequestHeaderField_Content_Type;
	} else if ( stringEquality(fieldName, HTTPRequestHeaderFieldName_Date) ) {
		return HTTPRequestHeaderField_Date;
	} else if ( stringEquality(fieldName, HTTPRequestHeaderFieldName_Expect) ) {
		return HTTPRequestHeaderField_Expect;
	} else if ( stringEquality(fieldName, HTTPRequestHeaderFieldName_From) ) {
		return HTTPRequestHeaderField_From;
	} else if ( stringEquality(fieldName, HTTPRequestHeaderFieldName_Host) ) {
		return HTTPRequestHeaderField_Host;
	} else if ( stringEquality(fieldName, HTTPRequestHeaderFieldName_If_Match) ) {
		return HTTPRequestHeaderField_If_Match;
	} else if ( stringEquality(fieldName, HTTPRequestHeaderFieldName_If_Modified_Since) ) {
		return HTTPRequestHeaderField_If_Modified_Since;
	} else if ( stringEquality(fieldName, HTTPRequestHeaderFieldName_If_None_Match) ) {
		return HTTPRequestHeaderField_If_None_Match;
	} else if ( stringEquality(fieldName, HTTPRequestHeaderFieldName_If_Range) ) {
		return HTTPRequestHeaderField_If_Range;
	} else if ( stringEquality(fieldName, HTTPRequestHeaderFieldName_If_Unmodified_Since) ) {
		return HTTPRequestHeaderField_If_Unmodified_Since;
	} else if ( stringEquality(fieldName, HTTPRequestHeaderFieldName_Max_Forwards) ) {
		return HTTPRequestHeaderField_Max_Forwards;
	} else if ( stringEquality(fieldName, HTTPRequestHeaderFieldName_Origin) ) {
		return HTTPRequestHeaderField_Origin;
	} else if ( stringEquality(fieldName, HTTPRequestHeaderFieldName_Pragma) ) {
		return HTTPRequestHeaderField_Pragma;
	} else if ( stringEquality(fieldName, HTTPRequestHeaderFieldName_Proxy_Authorization) ) {
		return HTTPRequestHeaderField_Proxy_Authorization;
	} else if ( stringEquality(fieldName, HTTPRequestHeaderFieldName_Pragma) ) {
		return HTTPRequestHeaderField_Pragma;
	} else if ( stringEquality(fieldName, HTTPRequestHeaderFieldName_Proxy_Authorization) ) {
		return HTTPRequestHeaderField_Proxy_Authorization;
	} else if ( stringEquality(fieldName, HTTPRequestHeaderFieldName_Range) ) {
		return HTTPRequestHeaderField_Range;
	} else if ( stringEquality(fieldName, HTTPRequestHeaderFieldName_Referer) ) {
		return HTTPRequestHeaderField_Referer;
	} else if ( stringEquality(fieldName, HTTPRequestHeaderFieldName_TE) ) {
		return HTTPRequestHeaderField_TE;
	} else if ( stringEquality(fieldName, HTTPRequestHeaderFieldName_User_Agent) ) {
		return HTTPRequestHeaderField_User_Agent;
	} else if ( stringEquality(fieldName, HTTPRequestHeaderFieldName_Via) ) {
		return HTTPRequestHeaderField_Via;
	} else if ( stringEquality(fieldName, HTTPRequestHeaderFieldName_Warning) ) {
		return HTTPRequestHeaderField_Warning;
	}
	
	// Conversion failed.
	return -1;
}

const char *HTTPRequestHeaderFieldNameForField(HTTPRequestHeaderField field)
{
	switch (field) {
		case HTTPRequestHeaderField_Accept: return HTTPRequestHeaderFieldName_Accept;
		case HTTPRequestHeaderField_Accept_Charset: return HTTPRequestHeaderFieldName_Accept_Charset;
		case HTTPRequestHeaderField_Accept_Encoding: return HTTPRequestHeaderFieldName_Accept_Encoding;
		case HTTPRequestHeaderField_Accept_Language: return HTTPRequestHeaderFieldName_Accept_Language;
		case HTTPRequestHeaderField_Accept_Datetime: return HTTPRequestHeaderFieldName_Accept_Datetime;
		case HTTPRequestHeaderField_Authorization: return HTTPRequestHeaderFieldName_Authorization;
		case HTTPRequestHeaderField_Cache_Control: return HTTPRequestHeaderFieldName_Cache_Control;
		case HTTPRequestHeaderField_Connection: return HTTPRequestHeaderFieldName_Connection;
		case HTTPRequestHeaderField_Proxy_Connection: return HTTPRequestHeaderFieldName_Proxy_Connection;
		case HTTPRequestHeaderField_Cookie: return HTTPRequestHeaderFieldName_Cookie;
		case HTTPRequestHeaderField_Content_Length: return HTTPRequestHeaderFieldName_Content_Length;
		case HTTPRequestHeaderField_Content_MD5: return HTTPRequestHeaderFieldName_Content_MD5;
		case HTTPRequestHeaderField_Content_Type: return HTTPRequestHeaderFieldName_Content_Type;
		case HTTPRequestHeaderField_Date: return HTTPRequestHeaderFieldName_Date;
		case HTTPRequestHeaderField_Expect: return HTTPRequestHeaderFieldName_Expect;
		case HTTPRequestHeaderField_From: return HTTPRequestHeaderFieldName_From;
		case HTTPRequestHeaderField_Host: return HTTPRequestHeaderFieldName_Host;
		case HTTPRequestHeaderField_If_Match: return HTTPRequestHeaderFieldName_If_Match;
		case HTTPRequestHeaderField_If_Modified_Since: return HTTPRequestHeaderFieldName_If_Modified_Since;
		case HTTPRequestHeaderField_If_None_Match: return HTTPRequestHeaderFieldName_If_None_Match;
		case HTTPRequestHeaderField_If_Range: return HTTPRequestHeaderFieldName_If_Range;
		case HTTPRequestHeaderField_If_Unmodified_Since: return HTTPRequestHeaderFieldName_If_Unmodified_Since;
		case HTTPRequestHeaderField_Max_Forwards: return HTTPRequestHeaderFieldName_Max_Forwards;
		case HTTPRequestHeaderField_Origin: return HTTPRequestHeaderFieldName_Origin;
		case HTTPRequestHeaderField_Pragma: return HTTPRequestHeaderFieldName_Pragma;
		case HTTPRequestHeaderField_Proxy_Authorization: return HTTPRequestHeaderFieldName_Proxy_Authorization;
		case HTTPRequestHeaderField_Range: return HTTPRequestHeaderFieldName_Range;
		case HTTPRequestHeaderField_Referer: return HTTPRequestHeaderFieldName_Referer;
		case HTTPRequestHeaderField_TE: return HTTPRequestHeaderFieldName_TE;
		case HTTPRequestHeaderField_User_Agent: return HTTPRequestHeaderFieldName_User_Agent;
		case HTTPRequestHeaderField_Via: return HTTPRequestHeaderFieldName_Via;
		case HTTPRequestHeaderField_Warning: return HTTPRequestHeaderFieldName_Warning;
		default: return NULL;
	}
}
