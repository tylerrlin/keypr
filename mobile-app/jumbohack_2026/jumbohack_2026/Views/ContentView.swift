//
//  ContentView.swift
//  jumnohack_2026
//
//  Created by John Cha on 2/21/26.
//

import SwiftUI

struct ContentView: View {
    var body: some View {
        GeometryReader { geo in
            ZStack {
                AppColors.blackBackground
                    .ignoresSafeArea()
                
                VStack(spacing: 0) {
                    // Top 15% gold section
                    
                        
                    Text("keypr")
                        .font(.custom("KodeMono-Regular", size: 36))
                        .fontWeight(.bold)
                        .foregroundColor(AppColors.goldMainText)
                        .padding(.top, 25)
                    
                    Rectangle()
                        .fill(AppColors.blueAccent)
                        .frame(height: 2)
                        .padding(.top, 10)
                        .padding(.horizontal, 25)
                    
                    Spacer()
                    
                    
                }
                
            }
            .frame(height: .infinity)
        }
    }
}

struct ContentView_Previews: PreviewProvider {
    static var previews: some View {
        ContentView()
            .previewDevice("iPhone 14 Pro")
            .preferredColorScheme(.dark)
    }
}
