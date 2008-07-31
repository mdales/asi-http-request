//
//  ASIHTTPRequest.h
//
//  Created by Ben Copsey on 04/10/2007.
//  Copyright 2007-2008 All-Seeing Interactive. All rights reserved.
//
//  Portions are based on the ImageClient example from Apple:
//  See: http://developer.apple.com/samplecode/ImageClient/listing37.html

#import <Cocoa/Cocoa.h>
#import "ASIProgressDelegate.h"



@interface ASIHTTPRequest : NSOperation {
	
	//The url for this operation, should include GET params in the query string where appropriate
	NSURL *url; 
	
	//The delegate, you need to manage setting and talking to your delegate in your subclasses
	id delegate;
	
	//Parameters that will be POSTed to the url
	NSMutableDictionary *postData;
	
	//Files that will be POSTed to the url
	NSMutableDictionary *fileData;
	
	//Dictionary for custom request headers
	NSMutableDictionary *requestHeaders;
	
	//If useKeychainPersistance is true, network requests will attempt to read credentials from the keychain, and will save them in the keychain when they are successfully presented
	BOOL useKeychainPersistance;
	
	//If useSessionPersistance is true, network requests will save credentials and reuse for the duration of the session (until clearSession is called)
	BOOL useSessionPersistance;
	
	//When downloadDestinationPath is set, the result of this request will be downloaded to the file at this location
	//If downloadDestinationPath is not set, download data will be stored in memory
	NSString *downloadDestinationPath;
	
	//Used for writing data to a file when downloadDestinationPath is set
	NSOutputStream *outputStream;
	
	//When the request fails or completes successfully, complete will be true
	BOOL complete;
	
	//If an error occurs, error will contain an NSError
	NSError *error;
	
	//If an authentication error occurs, we give the delegate a chance to handle it, ignoreError will be set to true
	BOOL ignoreError;
	
	//Username and password used for authentication
	NSString *username;
	NSString *password;
	
	//Delegate for displaying upload progress (usually an NSProgressIndicator, but you can supply a different object and handle this yourself)
	NSObject <ASIProgressDelegate> *uploadProgressDelegate;
	
	//Delegate for displaying download progress (usually an NSProgressIndicator, but you can supply a different object and handle this yourself)
	NSObject <ASIProgressDelegate> *downloadProgressDelegate;

	// Whether we've seen the headers of the response yet
    BOOL haveExaminedHeaders;
	
	//Data we receive will be stored here
	CFMutableDataRef receivedData;
	
	//Used for sending and receiving data
    CFHTTPMessageRef request;	
	CFReadStreamRef readStream;
	
	// Authentication currently being used for prompting and resuming
    CFHTTPAuthenticationRef authentication;  
	
	// Credentials associated with the authentication (reused until server says no)
	//CFMutableDictionaryRef credentials; 

	//Size of the response
	double contentLength;

	//Size of the POST payload
	double postLength;	
	
	//The total amount of downloaded data
	double totalBytesRead;
	
	//Last amount of data read (used for incrementing progress)
	double lastBytesRead;
	//Last amount of data sent (used for incrementing progress)
	double lastBytesSent;
	
	//Realm for authentication when credentials are required
	NSString *authenticationRealm;

	//This lock will block the request until the delegate supplies authentication info
	NSConditionLock *authenticationLock;
	
	//Called on the delegate when the request completes successfully
	SEL didFinishSelector;
	
	//Called on the delegate when the request fails
	SEL didFailSelector;
	
	NSDictionary *responseHeaders;
	NSMutableDictionary *requestCredentials;
	
}

#pragma mark init / dealloc

// Should be an HTTP or HTTPS url, may include username and password if appropriate
- (id)initWithURL:(NSURL *)newURL;

#pragma mark delegate configuration


#pragma mark setup request

//Add a custom header to the request
- (void)addRequestHeader:(NSString *)header value:(NSString *)value;

//Add a POST variable to the request
- (void)setPostValue:(id)value forKey:(NSString *)key;

//Add the contents of a local file as a POST variable to the request
- (void)setFile:(NSString *)filePath forKey:(NSString *)key;

// When set, username and password will be presented for HTTP authentication
- (void)setUsername:(NSString *)newUsername andPassword:(NSString *)newPassword;

#pragma mark get information about this request

- (BOOL)isFinished; //Same thing, for NSOperationQueues to read

// Get total amount of data received so far for this request
- (double)totalBytesRead;

// Returns the contents of the result as an NSString (not appropriate for binary data!)
- (NSString *)dataString;

#pragma mark request logic

// Start loading the request
- (void)loadRequest;

// Cancel loading and clean up
- (void)cancelLoad;

#pragma mark upload/download progress

// Called on main thread to update progress delegates
- (void)updateProgressIndicators;
- (void)resetUploadProgress:(NSNumber *)max;
- (void)updateUploadProgress;
- (void)resetDownloadProgress:(NSNumber *)max;
- (void)updateDownloadProgress;

#pragma mark handling request complete / failure

//Called when a request completes successfully - defaults to: @selector(requestFinished:)
- (void)requestFinished;

//Called when a request fails - defaults to: @selector(requestFailed:)
- (void)failWithProblem:(NSString *)problem;

#pragma mark http authentication stuff

// Reads the response headers to find the content length, and returns true if the request needs a username and password (or if those supplied were incorrect)
- (BOOL)readResponseHeadersReturningAuthenticationFailure;

// Unlock (unpause) the request thread so it can resume the request
// Should be called by delegates when they have populated the authentication information after an authentication challenge
- (void)retryWithAuthentication;

// Apply authentication information and resume the request after an authentication challenge
- (void)attemptToApplyCredentialsAndResume;

// Customise or overidde this to have a generic error for authentication failure
- (NSError *)authenticationError;

#pragma mark stream status handlers

// CFnetwork event handlers
- (void)handleNetworkEvent:(CFStreamEventType)type;
- (void)handleBytesAvailable;
- (void)handleStreamComplete;
- (void)handleStreamError;


#pragma mark keychain storage

//Save credentials to the keychain
+ (void)saveCredentials:(NSURLCredential *)credentials forHost:(NSString *)host port:(int)port protocol:(NSString *)protocol realm:(NSString *)realm;

//Return credentials from the keychain
+ (NSURLCredential *)savedCredentialsForHost:(NSString *)host port:(int)port protocol:(NSString *)protocol realm:(NSString *)realm;

//Remove credentials from the keychain
+ (void)removeCredentialsForHost:(NSString *)host port:(int)port protocol:(NSString *)protocol realm:(NSString *)realm;


@property (retain,readonly) NSURL *url;
@property (assign) id delegate;
@property (assign) NSObject *uploadProgressDelegate;
@property (assign) NSObject *downloadProgressDelegate;
@property (assign) BOOL useKeychainPersistance;
@property (assign) BOOL useSessionPersistance;
@property (retain) NSString *downloadDestinationPath;
@property (assign) SEL didFinishSelector;
@property (assign) SEL didFailSelector;
@property (retain,readonly) NSString *authenticationRealm;
@property (retain) NSError *error;
@property (assign,readonly) BOOL complete;
@property (retain) NSDictionary *responseHeaders;
@property (retain) NSDictionary *requestCredentials;

- (void)saveCredentialsToKeychain:(NSMutableDictionary *)newCredentials;
- (BOOL)applyCredentials:(NSMutableDictionary *)newCredentials;


@end