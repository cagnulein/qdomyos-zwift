/****************************************************************************
 **
 ** Copyright (C) 2021 The Qt Company Ltd.
 ** Contact: https://www.qt.io/licensing/
 **
 ** This file is part of the examples of the Qt Toolkit.
 **
 ** $QT_BEGIN_LICENSE:BSD$
 ** Commercial License Usage
 ** Licensees holding valid commercial Qt licenses may use this file in
 ** accordance with the commercial license agreement provided with the
 ** Software or, alternatively, in accordance with the terms contained in
 ** a written agreement between you and The Qt Company. For licensing terms
 ** and conditions see https://www.qt.io/terms-conditions. For further
 ** information use the contact form at https://www.qt.io/contact-us.
 **
 ** BSD License Usage
 ** Alternatively, you may use this file under the terms of the BSD license
 ** as follows:
 **
 ** "Redistribution and use in source and binary forms, with or without
 ** modification, are permitted provided that the following conditions are
 ** met:
 **   * Redistributions of source code must retain the above copyright
 **     notice, this list of conditions and the following disclaimer.
 **   * Redistributions in binary form must reproduce the above copyright
 **     notice, this list of conditions and the following disclaimer in
 **     the documentation and/or other materials provided with the
 **     distribution.
 **   * Neither the name of The Qt Company Ltd nor the names of its
 **     contributors may be used to endorse or promote products derived
 **     from this software without specific prior written permission.
 **
 **
 ** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 ** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 ** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 ** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 ** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 ** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 ** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 ** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 ** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 ** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 ** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
 **
 ** $QT_END_LICENSE$
 **
 ****************************************************************************/

package org.qtproject.qt.android.purchasing;

import java.util.ArrayList;
import java.util.List;

import android.app.Activity;
import android.content.Context;
import org.cagnulen.qdomyoszwift.QLog;

import com.android.billingclient.api.AcknowledgePurchaseParams;
import com.android.billingclient.api.AcknowledgePurchaseResponseListener;
import com.android.billingclient.api.BillingClient;
import com.android.billingclient.api.BillingClientStateListener;
import com.android.billingclient.api.BillingFlowParams;
import com.android.billingclient.api.BillingResult;
import com.android.billingclient.api.ConsumeParams;
import com.android.billingclient.api.ConsumeResponseListener;
import com.android.billingclient.api.PendingPurchasesParams;
import com.android.billingclient.api.ProductDetails;
import com.android.billingclient.api.ProductDetailsResponseListener;
import com.android.billingclient.api.Purchase;
import com.android.billingclient.api.Purchase.PurchaseState;
import com.android.billingclient.api.PurchasesResponseListener;
import com.android.billingclient.api.PurchasesUpdatedListener;
import com.android.billingclient.api.QueryProductDetailsParams;
import com.android.billingclient.api.QueryPurchasesParams;
import com.android.billingclient.api.QueryProductDetailsResult;


/***********************************************************************
 ** More info: https://developer.android.com/google/play/billing
 ** Add Dependencies below to build.gradle file:

dependencies {
    def billing_version = "8.0.0"
    implementation "com.android.billingclient:billing:$billing_version"
}

***********************************************************************/

public class InAppPurchase implements PurchasesUpdatedListener
{
    private Context m_context = null;
    private long m_nativePointer;
    private String m_publicKey = null;
    private int purchaseRequestCode;


    private BillingClient billingClient;

    public static final int RESULT_OK = BillingClient.BillingResponseCode.OK;
    public static final int RESULT_USER_CANCELED = BillingClient.BillingResponseCode.USER_CANCELED;
	 public static final String TYPE_INAPP = BillingClient.ProductType.INAPP;
	 public static final String TYPE_SUBS = BillingClient.ProductType.SUBS;
    public static final String TAG = "InAppPurchase";

    // Should be in sync with InAppTransaction::FailureReason
    public static final int FAILUREREASON_NOFAILURE    = 0;
    public static final int FAILUREREASON_USERCANCELED = 1;
    public static final int FAILUREREASON_ERROR        = 2;

	 public InAppPurchase()
    {
		  //m_context = context;
		  //m_nativePointer = nativePointer;
    }

    public void initPointer(Context context, long nativePointer)
	 {
		 m_context = context;
		 m_nativePointer = nativePointer;
	 }

    public void initializeConnection(){
		  QLog.w(TAG, "initializeConnection start");
        PendingPurchasesParams pendingPurchasesParams = PendingPurchasesParams.newBuilder()
                .enableOneTimeProducts()
                .build();
        billingClient = BillingClient.newBuilder(m_context)
                .enablePendingPurchases(pendingPurchasesParams)
                .setListener(this)
                .build();
        billingClient.startConnection(new BillingClientStateListener() {
            @Override
            public void onBillingSetupFinished(BillingResult billingResult) {
					QLog.w(TAG, "onBillingSetupFinished");
                if (billingResult.getResponseCode() == RESULT_OK) {
                    purchasedProductsQueried(m_nativePointer);
						} else {
					     QLog.w(TAG, "onBillingSetupFinished error!" + billingResult.getResponseCode());
					 }
            }

            @Override
            public void onBillingServiceDisconnected() {
                QLog.w(TAG, "Billing service disconnected");
            }
        });
    }

    @Override
    public void onPurchasesUpdated(BillingResult billingResult, List<Purchase> purchases) {

        int responseCode = billingResult.getResponseCode();
        QLog.d(TAG, "onPurchasesUpdated called. Response code: " + responseCode + ", Debug message: " + billingResult.getDebugMessage());

        if (purchases == null) {
            QLog.e(TAG, "Purchase failed: Data missing from result (purchases is null)");
            purchaseFailed(purchaseRequestCode, FAILUREREASON_ERROR, "Data missing from result");
            return;
        }

        if (billingResult.getResponseCode() == RESULT_OK) {
            QLog.d(TAG, "Purchase successful, handling " + purchases.size() + " purchases");
            handlePurchase(purchases);
        } else if (responseCode == RESULT_USER_CANCELED) {
            QLog.d(TAG, "Purchase cancelled by user");
            purchaseFailed(purchaseRequestCode, FAILUREREASON_USERCANCELED, "");
        } else {
            String errorString = getErrorString(responseCode);
            QLog.e(TAG, "Purchase failed with error: " + errorString + " (code: " + responseCode + ")");
            purchaseFailed(purchaseRequestCode, FAILUREREASON_ERROR, errorString);
        }
    }

    //Get list of purchases from onPurchasesUpdated
    private void handlePurchase(List<Purchase> purchases) {

        for (Purchase purchase : purchases) {
            try {
                if (m_publicKey != null && !Security.verifyPurchase(m_publicKey, purchase.getOriginalJson(), purchase.getSignature())) {
                    purchaseFailed(purchaseRequestCode, FAILUREREASON_ERROR, "Signature could not be verified");
                    return;
                }
                int purchaseState = purchase.getPurchaseState();
                if (purchaseState != PurchaseState.PURCHASED) {
                    purchaseFailed(purchaseRequestCode, FAILUREREASON_ERROR, "Unexpected purchase state in result");
                    return;
                }
            } catch (Exception e) {
                e.printStackTrace();
                purchaseFailed(purchaseRequestCode, FAILUREREASON_ERROR, e.getMessage());
            }
            purchaseSucceeded(purchaseRequestCode, purchase.getSignature(), purchase.getOriginalJson(), purchase.getPurchaseToken(), purchase.getOrderId(), purchase.getPurchaseTime());
            AcknowledgePurchaseParams acknowledgePurchaseParams =
                        AcknowledgePurchaseParams.newBuilder()
                                .setPurchaseToken(purchase.getPurchaseToken())
                                .build();
            billingClient.acknowledgePurchase(acknowledgePurchaseParams,
                new AcknowledgePurchaseResponseListener()
                {
                    @Override
                    public void onAcknowledgePurchaseResponse(BillingResult billingResult)
                    {
                        QLog.d(TAG, "Purchase acknowledged  ");
                    }
                }
            );
        }
    }

    public void queryDetails(final String[] productIds) {
		  QLog.d(TAG, "queryDetails: start");
        int index = 0;
		  QLog.d(TAG, "queryDetails: productIds.length " + productIds.length);
        while (index < productIds.length) {
            List<String> productIdList = new ArrayList<>();
            for (int i = index; i < Math.min(index + 20, productIds.length); ++i) {
                productIdList.add(productIds[i]);					 
            }
            index += productIdList.size();

            List<QueryProductDetailsParams.Product> productList = new ArrayList<>();
            for (String productId : productIdList) {
                productList.add(
                    QueryProductDetailsParams.Product.newBuilder()
                        .setProductId(productId)
                        .setProductType(TYPE_SUBS)
                        .build());
            }
            QueryProductDetailsParams params = QueryProductDetailsParams.newBuilder()
                    .setProductList(productList)
                    .build();
            billingClient.queryProductDetailsAsync(params,
                    (billingResult, productDetailsResult) -> {
                            List<ProductDetails> productDetailsList = productDetailsResult.getProductDetailsList();
                            int responseCode = billingResult.getResponseCode();
									 QLog.d(TAG, "onProductDetailsResponse: responseCode " + responseCode);

                            if (responseCode != RESULT_OK) {
                                QLog.e(TAG, "queryDetails: Couldn't retrieve product details.");
                                return;
                            }
                            if (productDetailsList == null || productDetailsList.isEmpty()) {
                                QLog.e(TAG, "queryDetails: No details list in response.");
                                return;
                            }

								    QLog.d(TAG, "onProductDetailsResponse: productDetailsList " + productDetailsList);
                            for (ProductDetails productDetails : productDetailsList) {
                                try {
                                    String queriedProductId = productDetails.getProductId();
                                    String queriedPrice = "";
                                    String queriedTitle = productDetails.getTitle();
                                    String queriedDescription = productDetails.getDescription();
                                    
                                    // Get price from subscription offer details
                                    if (productDetails.getSubscriptionOfferDetails() != null && !productDetails.getSubscriptionOfferDetails().isEmpty()) {
                                        queriedPrice = productDetails.getSubscriptionOfferDetails().get(0).getPricingPhases().getPricingPhaseList().get(0).getFormattedPrice();
                                    }
                                    registerProduct(m_nativePointer,
                                            queriedProductId,
                                            queriedPrice,
                                            queriedTitle,
                                            queriedDescription);
                                } catch (Exception e) {
                                    e.printStackTrace();
                                }
                            }
                    });


            queryPurchasedProducts(productIdList);
        }
    }

    //Launch Google purchasing screen
    public void launchBillingFlow(String identifier, final int requestCode){

        purchaseRequestCode = requestCode;
        List<QueryProductDetailsParams.Product> productList = new ArrayList<>();
        productList.add(
            QueryProductDetailsParams.Product.newBuilder()
                .setProductId(identifier)
                .setProductType(TYPE_SUBS)
                .build());
        QueryProductDetailsParams params = QueryProductDetailsParams.newBuilder()
                .setProductList(productList)
                .build();
        billingClient.queryProductDetailsAsync(params,
                (billingResult, productDetailsResult) -> {
                        List<ProductDetails> productDetailsList = productDetailsResult.getProductDetailsList();

                        if (billingResult.getResponseCode() != RESULT_OK) {
                            QLog.e(TAG, "Unable to launch Google Play purchase screen. Response code: " + billingResult.getResponseCode() + ", Debug message: " + billingResult.getDebugMessage());
                            String errorString = getErrorString(requestCode);
                            purchaseFailed(requestCode, FAILUREREASON_ERROR, errorString);
                            return;
                        }
                        else if (productDetailsList == null || productDetailsList.isEmpty()){
                            purchaseFailed(purchaseRequestCode, FAILUREREASON_ERROR, "Data missing from result");
                            return;
                        }

                        ProductDetails productDetails = productDetailsList.get(0);
                        BillingFlowParams.ProductDetailsParams.Builder productDetailsParamsBuilder = BillingFlowParams.ProductDetailsParams.newBuilder()
                                .setProductDetails(productDetails);
                        
                        // For subscriptions, we need to set the offer token
                        if (productDetails.getSubscriptionOfferDetails() != null && !productDetails.getSubscriptionOfferDetails().isEmpty()) {
                            String offerToken = productDetails.getSubscriptionOfferDetails().get(0).getOfferToken();
                            QLog.d(TAG, "Setting offer token for subscription: " + offerToken);
                            productDetailsParamsBuilder.setOfferToken(offerToken);
                        } else {
                            QLog.w(TAG, "No subscription offer details found for product: " + identifier);
                        }
                        
                        BillingFlowParams.ProductDetailsParams productDetailsParams = productDetailsParamsBuilder.build();
                        
                        BillingFlowParams purchaseParams = BillingFlowParams.newBuilder()
                                .setProductDetailsParamsList(java.util.Arrays.asList(productDetailsParams))
                                .build();

                        //Results will be delivered to onPurchasesUpdated
                        billingClient.launchBillingFlow((Activity) m_context, purchaseParams);
                });
    }

    public void consumePurchase(String purchaseToken){

        ConsumeResponseListener listener = new ConsumeResponseListener() {
            @Override
            public void onConsumeResponse(BillingResult billingResult, String purchaseToken) {
                if (billingResult.getResponseCode() != RESULT_OK) {
                    QLog.e(TAG, "Unable to consume purchase. Response code: " + billingResult.getResponseCode());
                }
            }
        };
        ConsumeParams consumeParams =
                ConsumeParams.newBuilder()
                        .setPurchaseToken(purchaseToken)
                        .build();
        billingClient.consumeAsync(consumeParams, listener);
    }

    public void acknowledgeUnlockablePurchase(String purchaseToken){

        AcknowledgePurchaseParams acknowledgePurchaseParams = AcknowledgePurchaseParams.newBuilder()
                .setPurchaseToken(purchaseToken)
                .build();

        AcknowledgePurchaseResponseListener acknowledgePurchaseResponseListener = new AcknowledgePurchaseResponseListener() {
            @Override
            public void onAcknowledgePurchaseResponse(BillingResult billingResult) {
                if (billingResult.getResponseCode() != RESULT_OK){
                    QLog.e(TAG, "Unable to acknowledge purchase. Response code: " + billingResult.getResponseCode());
                }
            }
        };
        billingClient.acknowledgePurchase(acknowledgePurchaseParams, acknowledgePurchaseResponseListener);
    }

    public void queryPurchasedProducts(final List<String> productIdList) {

        QueryPurchasesParams queryPurchasesParams = QueryPurchasesParams.newBuilder()
                .setProductType(TYPE_SUBS)
                .build();
        billingClient.queryPurchasesAsync(queryPurchasesParams, new PurchasesResponseListener() {
            @Override
            public void onQueryPurchasesResponse(BillingResult billingResult, List<Purchase> list) {
                for (Purchase purchase : list) {

                    if (productIdList.contains(purchase.getProducts().get(0))) {
                        registerPurchased(m_nativePointer,
                                purchase.getProducts().get(0),
                                purchase.getSignature(),
                                purchase.getOriginalJson(),
                                purchase.getPurchaseToken(),
                                "", // getDeveloperPayload() is deprecated
                                purchase.getPurchaseTime());
                    }
                }
            }
        });
    }

    private String getErrorString(int responseCode){
        String errorString;
        switch (responseCode) {
            case BillingClient.BillingResponseCode.BILLING_UNAVAILABLE: errorString = "Billing unavailable"; break;
            case BillingClient.BillingResponseCode.ITEM_UNAVAILABLE: errorString = "Item unavailable"; break;
            case BillingClient.BillingResponseCode.DEVELOPER_ERROR: errorString = "Developer error"; break;
            case BillingClient.BillingResponseCode.ERROR: errorString = "Fatal error occurred"; break;
            case BillingClient.BillingResponseCode.ITEM_ALREADY_OWNED: errorString = "Item already owned"; break;
            case BillingClient.BillingResponseCode.ITEM_NOT_OWNED: errorString = "Item not owned"; break;
            default: errorString = "Unknown billing error " + responseCode; break;
        };
        return errorString;
    }

    public void setPublicKey(String publicKey)
    {
        m_publicKey = publicKey;
    }

    private void purchaseFailed(int requestCode, int failureReason, String errorString)
    {
        purchaseFailed(m_nativePointer, requestCode, failureReason, errorString);
    }


    private void purchaseSucceeded(int requestCode,
                                   String signature,
                                   String purchaseData,
                                   String purchaseToken,
                                   String orderId,
                                   long timestamp)
    {
        purchaseSucceeded(m_nativePointer, requestCode, signature, purchaseData, purchaseToken, orderId, timestamp);
    }

    private native static void queryFailed(long nativePointer, String productId);
    private native static void purchasedProductsQueried(long nativePointer);
    private native static void registerProduct(long nativePointer,
                                               String productId,
                                               String price,
                                               String title,
                                               String description);
    private native static void purchaseFailed(long nativePointer,
                                              int requestCode,
                                              int failureReason,
                                              String errorString);
    private native static void purchaseSucceeded(long nativePointer,
                                                 int requestCode,
                                                 String signature,
                                                 String data,
                                                 String purchaseToken,
                                                 String orderId,
                                                 long timestamp);
    private native static void registerPurchased(long nativePointer,
                                                 String identifier,
                                                 String signature,
                                                 String data,
                                                 String purchaseToken,
                                                 String orderId,
                                                 long timestamp);
}
