//
//  PayPalHereSDKSingleton.h
//  PayPalHereSDK
//
//  Created by Schneider, Griffin on 4/6/15.
//  Copyright (c) 2015 PayPal. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "PPHCardReaderManager.h"
#import "PPHLocalManager.h"
#import "PPHPaymentProcessor.h"
#import "PPHAccessController.h"
#import "PPHTransactionManager.h"
#import "PPHNetworkRequestDelegate.h"
#import "PPHAnalyticsDelegate.h"
#import "PPHLoggingDelegate.h"

@class PPHMerchantInfo;
@class PPHAccessController;

/*!
 * Enum for the service endpoint to connect to.
 */
typedef NS_ENUM(NSInteger, PPHSDKServiceType) {
    /*!
     * SDK points to Live.
     */
    ePPHSDKServiceType_Live,
    /*!
     * SDK points to Sandbox.
     */
    ePPHSDKServiceType_Sandbox,
};

typedef NS_ENUM(NSInteger, PPHInitResultType) {
    /*!
     * If the merchant setup was successful.
     */
    ePPHInitResultSuccess,
    
    /*!
     * Init failed due to insufficient information in the PPHAccessAccount object. This usually happens when the application attempts to initialize
     * the SDK with a PPHAccessAccount object that does not have a valid access token or refresh url
     */
    ePPHInitResultFailedInsufficientCredentials,
    
    /*!
     * Init failed because of an error we encountered when retrieving merchant info. Please see PPHError object return from this call for more information
     */
    ePPHInitResultFailed
};


typedef void (^PPHInitializationCompletionHandler)(PPHInitResultType status, PPHError *error, PPHMerchantInfo *info);


/*!
 * The main object through which you can access other resources provided by the SDK. This is a singleton
 * and as such has global context. This object serves three main purposes:
 * a) Initialization of the SDK
 * b) Set global configurations - for ex: you can set a network delegate to listen for networking calls
 * c) Access Shared resources - the other objects in the SDK such as TransactionManager, Card Reader Manager
 * can be accessed from this object
 *
 * INITIALIZATION: This SDK comes to life once the initialization sequence gets rolling. In order to initialize
 * the SDK you will need to have an access token, expiry for the token and a refresh url. Your application needs
 * perform the OAuth login at the end of which you will have these values generated by Login With PayPal.
 * When the SDK is intialized, we internally go about setting up a PPHMerchantInfo object from the credentials
 * provided. Think of this object as the central entity around which actions are performed. As a merchant facing
 * application you are in essence performing actions on behalf of the merchant and this simply reflects that.
 * The initialization sequence fires a completion handler when it is done and it is IMPORTANT that you - the
 * application developer - check the status of initialization and handle errors meaningfully. There are developer
 * facing error messages to help ensure that the correct set of values are passed. As an example: we return
 * ePPHInitResultFailedInsufficientCredentials when you attempt initialization without a valid access token or
 * refresh url.
 * 
 * Once intialization is complete, you can grab the shared manager objects to perform transactions using the SDK.
 * Start with the PPHTransactionManager which can be retrieved using [PayPalHereSDK sharedTransactionManager]. The
 * transaction manager itself is well documented and will walk you through the steps needed to be able to take
 * payments with the PayPal Here SDK. Happy coding!
 */
@interface PayPalHereSDK : NSObject

/*
 * Shared service providers for single-use-per-app type of capabilities
 */

/*!
 * The interface to interact with the card reader
 */
+(PPHCardReaderManager*) sharedCardReaderManager;
/*!
 * The interface to interact with checkin services and merchant locations
 */
+(PPHLocalManager*) sharedLocalManager;
/*!
 * The interface to process payments with card swipe, keyed in/scanned, and checkins
 */
+(PPHPaymentProcessor*) sharedPaymentProcessor;
/*!
 * A helper to establish OAuth credentials on behalf of your application for a merchant.
 */
+(PPHAccessController*) sharedAccessController DEPRECATED_ATTRIBUTE;

/*!
 * A way to do payments & refunds in a stateful way
 */
+(PPHTransactionManager*) sharedTransactionManager;

/*!
 * Should you wish to handle your own network requests, you can set this singleton
 * which the PayPal Here SDK will call whenever a network request needs to go out.
 * @param delegate an object implementing PPHNetworkRequestDelegate which will be in charge of executing requests
 */
+(void)setNetworkDelegate: (id<PPHNetworkRequestDelegate>) delegate;
/*!
 * The current network request delegate.
 */
+(id<PPHNetworkRequestDelegate>)networkDelegate;

/*!
 * Should you wish to handle your own analytics, you can set this singleton.
 * Be warned if you don't also let us do our own analytics we will not be able
 * to help diagnose aggregate issues for our SDK in your application.
 * @param delegate an object implementing PPHAnalyticsDelegate which will be in charge of reporting events
 */
+(void)setAnalyticsDelegate: (id<PPHAnalyticsDelegate>) delegate;
/*!
 * The current analytics delegate.
 */
+(id<PPHAnalyticsDelegate>)analyticsDelegate;

/*!
 * Should you wish to receive internal log messages, set this delegate. See
 * PPHLogingDelegate for a warning about performance impact.
 * @param delegate an object implementing one or more methods of
 * the PPHLoggingDelegate
 */
+(void)setLoggingDelegate: (id<PPHLoggingDelegate>) delegate;
/*!
 * The current logging delegate.
 */
+(id<PPHLoggingDelegate>)loggingDelegate;

/*!
 * The currently active merchant account
 */
+(PPHMerchantInfo*)activeMerchant;

/*!
 * This is now the de-facto way to initialize the PPH SDK. The current API - setActiveMerchant 
 * - will be marked for deprecation and will be removed at some point in the future.
 * As before, this initialization sequence will ensure that we persist certain merchant information
 * in secure storage. This allows the PPH SDK to re-use this information - such as email id - when such
 * info is not available through our API calls to backend.
 * 
 * HOW THIS METHOD WORKS: Once login is complete, you - the application - should have the following information:
 * a) An access token - this is a limited validity token that usually expires after 8 hours
 * b) Refresh Url - an endpoint that we can make a GET request to to fetch a new access token. When the access token expires
 * we will automatically invoke this endpoint to refresh the access token
 * c) Expiry time - this number usually indicates the time left (in seconds) before the access token expires.
 * You - the application - provide us these 3 pieces of data to the SDK and we in turn use this to set up some basic
 * information we need to know in order to make payment flows possible. For ex: we need to determine the merchant's
 * currency code to properly set up transactions. During initialization we fetch this - and other - details about the merchant
 * from our databases and create a PPHMerchantInfo object that we then return to the application in the completion handler.
 * The application is able to make some custom changes to non read-only properties of the merchant object and at the same
 * time this allows the SDK to ensure we have the minimum set of information we need to be able to process transactions.
 * NOTE: You MUST wait for the completion handler to fire AND check the status (PPHInitResultType) to determine that the
 * initialization was successful before attempting to proceed with the transaction flow.
 *
 * @param access_token The Access token returned to you from PayPal Access. You will get this after the OAuth flow is completed
 * @param refresh_url Also obtained after the OAuth flow. Once the merchant has authorized your application to make transactions on
 * your behalf you will be able to get the access token and the refresh url from PayPal Access endpoint
 * @param tokenExpiryOrNil this is a nice to have and will allow the SDK to predictively pre-fetch a new token when the current token is about
 * to expire. Consider this case: it is possible - although unlikely - that the validity of the access token can expire between the time
 * your application starts a new transaction and the time it actually submits the transaction to PayPal. In this case, we will re-attempt the 
 * transaction automatically after refreshing the access token but this extra call will result in a longer time taken and hence
 * a poor user experience. Setting a valid expiry time will help mitigate this scenario
 * @param completionHandler The handler that will fire once initialization is done. Your application MUST check the PPHInitResultType to ensure
 * there were no errors.
 */
+(void)setupWithCredentials:(NSString *)access_token refreshUrl:(NSString *)refresh_url tokenExpiryOrNil:(NSString *)expiry thenCompletionHandler:(PPHInitializationCompletionHandler) completionHandler;

/*!
 * Parse composite token string like the one returned from https://github.com/djMax/paypal-retail-node and initialize the SDK with it.
 *
 * @param compositeString The all powerful composit token string in the format: "sdk_token=[sandbox|live]:<base 64 encoded JSON object in the format of [authToken, expiry, refreshURL]>"
 * @param completionHandler The handler that will fire once initialization is done.
 */
+(void)setupWithCompositeTokenString:(NSString *)compositeString thenCompletionHandler:(PPHInitializationCompletionHandler)completionHandler;


/*!
 * Set the currently active merchant for which all payment operations will be done.
 * We will persist this merchant information to secure storage.  When your app runs
 * again and you call setActiveMerchant with a merchantId we've seen before we'll 
 * automatically check the saved merchant info for that merchantId.  If you pass in 
 * a nil or sparsely filled out merchant object we'll use the values we pulled from
 * storage.
 *
 * IMPORTANT: you must wait for the completion hander to fire and for successful
 * initialization otherwise you will not be able to do transactions or other API calls
 * successfully.
 *
 * @param merchant The merchant information including OAuth credentials
 * @param merchantId a value which we'll use to uniquely identify this merchant on this device so
 * we can save/fetch this specific merchant's information from local secure storage.
 * If more than one merchant logs into this system you should make sure you provide
 * different values.  This id belongs to the app and is not generated by paypal
 * nor sent down to the service.
 * @param completionHandler The handler to be called when the merchant setup has completed
 * (setup includes a call to the server to verify the token and retrieve user information)
 */

+(void)setActiveMerchant:(PPHMerchantInfo*)merchant withMerchantId:(NSString*)merchantId completionHandler: (PPHAccessCompletionHandler) completionHandler DEPRECATED_ATTRIBUTE;

/*!
 * Returns YES if we have the access we need to device location information
 * (we ask for significant location changes so long as a transaction is in progress,
 * or for general location updates if the device does not support significant updates).
 *
 * You will not be able to complete transactions without location services enabled.
 */
+(BOOL)hasLocationAccess;

/*!
 * DEPRECATED
 * Calls "- (void)startWatchingLocation"
 * Returns YES.
 */
+(BOOL)askForLocationAccess;

/*!
 * Allow the SDK to begin watching the location to attach to payments. In the future allowing
 * the SDK to obtain a location fix will be a requirement for taking payments. Location monitoring
 * is automatically stopped and restarted when the app is suspended if it is being watched.
 */
+(void)startWatchingLocation;

/*!
 * Disallow the SDK from getting location updates.
 */
+(void)stopWatchingLocation;

/*!
 * For TEST purposes, you can set the service URL used for requests in the PayPal Here SDK
 *
 * DEPRECATED: Use the selectEnvironmentWithType method which allows selection between available
 * services.
 *
 * @param url The base URL (essentially https://stagename/) for your non-live environment.
 */
+(void)setBaseAPIURL: (NSURL*) url DEPRECATED_ATTRIBUTE;

/*!
 * Used to select between PPHSDKServiceTypes - currently just Live or Sandbox.  
 * 
 * By default the SDK will run against Live, no need to call this method if you'd
 * like to use the Live service.
 *
 * However, if you want the SDK to run against PayPal's Sandbox environment,
 * or you wish to point the SDK back at Live, you can use this method.
 *
 * @param serviceType The service to connect to.
 */
+(void)selectEnvironmentWithType: (PPHSDKServiceType) serviceType;

/*!
 * The version of the SDK currently in use
 */
+(NSString*) sdkVersion;

/*!
 * The partner referrer code.
 */
+(NSString*) referrerCode;

/*!
 * Set the Partner Referrer code that is obtained after sigining up with PayPalHere.
 * NOTE: If the value is set in here, it would be automatically set within the invoice.
 * If not, you would need to feed in same the information within the invoice object.
 * @param referrerCode the referrer code that is obtained once a partner registers with PayPalHere.
 */
+(void) setReferrerCode: (NSString*) referrerCode;

/*!
 * Returns the current base URL
 */
+(NSString *)baseURL;

#ifdef DEBUG
+(void)sendStackTrace;
#endif

@end
