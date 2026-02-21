//
//  ContentView.swift
//  jumnohack_2026
//
//  Created by John Cha on 2/21/26.
//

import SwiftUI

struct ContentView: View {
    @State private var responseText = "Press the button"
    private let BACKEND_SERVER_URL = "10.243.46.233:8000"

    var body: some View {
        VStack(spacing: 20) {
            Text("hey what's going on")
            Text(responseText)
                .padding()
            
            Button("Send Request") {
                sanityCheck()
                sendRequest()
            }
            .padding()
            .background(Color.blue)
            .foregroundColor(Color.red)
            .cornerRadius(8)
        }
    }
    func sanityCheck() {
        print("The app should be up and running")
    }
    func sendRequest() {
        // Make sure this matches your local network IP if testing on a real phone
        guard let url = URL(string: "http://" + BACKEND_SERVER_URL + "/message") else { return }
        
        let task = URLSession.shared.dataTask(with: url) { data, _, error in
            if let data = data {
                if let json = try? JSONSerialization.jsonObject(with: data) as? [String: String],
                   let message = json["message"] {
                    DispatchQueue.main.async {
                        responseText = message
                    }
                }
            } else if let error = error {
                DispatchQueue.main.async {
                    responseText = "Error: \(error.localizedDescription)"
                }
            }
        }
        task.resume()
    }
}
