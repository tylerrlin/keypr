//
//  HistoryView.swift
//  jumbohack_2026
//
//  Created by Elizabeth Ngai on 2/21/26.
//

import SwiftUI

struct HistoryEntry: Identifiable {
    let id = UUID()
    let deviceName: String
    let status: String
    let date: String
    let time: String
    let approved: Bool
}

struct HistoryView: View {
    
    let entries: [HistoryEntry] = [
        HistoryEntry(deviceName: "E's Macbook Pro", status: "request approved", date: "02/21/26", time: "6:30PM", approved: true),
        HistoryEntry(deviceName: "E's Macbook Pro", status: "request approved", date: "02/21/26", time: "4:30PM", approved: true),
        HistoryEntry(deviceName: "E's Macbook Pro", status: "request declined", date: "02/21/26", time: "4:20PM", approved: false),
    ]
    
    var body: some View {
        VStack(alignment: .leading, spacing: 0) {
            Text("History")
                .font(.custom("KodeMono-Regular", size: 32))
                .foregroundColor(.white)
                .padding(.bottom, 10)
                .bold()
                .frame(maxWidth: .infinity, alignment: .center)
            ForEach(entries) { entry in
                HStack(alignment: .top) {
                    // Left column
                    VStack(alignment: .leading, spacing: 4) {
                        Text(entry.deviceName)
                            .foregroundColor(AppColors.whiteSubtext)
                            .font(.custom("KodeMono-Regular", size: 16))
                        Text(entry.status)
                            .foregroundColor(entry.approved ? AppColors.blueAccent : .red)
                            .font(.custom("KodeMono-Regular", size: 14))
                    }
                    
                    Spacer()
                    
                    // Right column
                    VStack(alignment: .trailing, spacing: 4) {
                        Text(entry.date)
                            .foregroundColor(AppColors.blueAccent)
                            .font(.custom("KodeMono-Regular", size: 16))
                        Text(entry.time)
                            .foregroundColor(AppColors.blueAccent)
                            .font(.custom("KodeMono-Regular", size: 16))
                    }
                }
                .padding(.vertical, 12)
                
                Divider()
                    .overlay(AppColors.whiteSubtext.opacity(0.4))
            }
            
            Spacer()
        }
        .padding(.horizontal, 25)
        .padding(.top, 16)
    }
}
