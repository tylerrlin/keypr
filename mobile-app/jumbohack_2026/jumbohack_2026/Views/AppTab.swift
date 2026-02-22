//
//  AppTab.swift
//  jumbohack_2026
//
//  Created by Elizabeth Ngai on 2/21/26.
//

import Foundation

enum AppTab: String, CaseIterable, Identifiable {
    case status, history, devices
    var id: String { rawValue }

    var title: String {
        switch self {
        case .status: return "status"
        case .history: return "history"
        case .devices: return "devices"
        }
    }

    var iconAssetName: String {
        switch self {
        case .status: return "icon_status"
        case .history: return "icon_history"
        case .devices: return "icon_devices"
        }
    }
}
