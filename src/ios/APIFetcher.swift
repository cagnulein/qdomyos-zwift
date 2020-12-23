//
//  APIFetcher.swift
//  ElecDemo
//
//  Created by NhatHM on 8/13/19.
//  Copyright © 2019 GST.PID. All rights reserved.
//

import Foundation

class APIFetcher {
    let defaultSession = URLSession(configuration: URLSessionConfiguration.default)
    var dataTask: URLSessionDataTask?
    
    func postHeartRateToServer(url: String, completion: @escaping (String?) -> ()) {
        
        let url: URL = URL(string: url)!
        
        dataTask = defaultSession.dataTask(with: url) { data, response, error in
            if let error = error {
                print(error.localizedDescription)
                completion(nil)
            } else if let httpResponse = response as? HTTPURLResponse {
                if httpResponse.statusCode == 200 {
                    completion("Sucess get data from server")
                } else {
                    print("\(httpResponse.statusCode)")
                    completion(nil)
                }
            }
        }
        
        dataTask?.resume()
    }
}
