//
//  ComplicationController.swift
//  ElecDemo WatchKit Extension
//
//  Created by NhatHM on 8/9/19.
//  Copyright © 2019 GST.PID. All rights reserved.
//

import ClockKit


class ComplicationController: NSObject, CLKComplicationDataSource {
    
    // MARK: - Timeline Configuration
    
    private func templateForComplication(complication: CLKComplication) -> CLKComplicationTemplate? {
        // Init default output:
        var template: CLKComplicationTemplate? = nil
        
        // Graphic Complications are only availably since watchOS 5.0:
        if #available(watchOSApplicationExtension 5.0, *) {
            // NOTE: Watch faces that support graphic templates are available only on Apple Watch Series 4 or later. So the binary on older devices (e.g. Watch Series 3) will not contain the images.
            if complication.family == .graphicCircular {
                let imageTemplate = CLKComplicationTemplateGraphicCircularImage()
                // Check if asset exists, to prevent crash on non-supported devices:
                if let fullColorImage = UIImage(named: "Complication/Graphic Circular") {
                    let imageProvider = CLKFullColorImageProvider.init(fullColorImage: fullColorImage)
                    imageTemplate.imageProvider = imageProvider
                    template = imageTemplate
                }
            }
            else if complication.family == .graphicCorner {
                let imageTemplate = CLKComplicationTemplateGraphicCornerCircularImage()
                // Check if asset exists, to prevent crash on non-supported devices:
                if let fullColorImage = UIImage(named: "Complication/Graphic Corner") {
                    let imageProvider = CLKFullColorImageProvider.init(fullColorImage: fullColorImage)
                    imageTemplate.imageProvider = imageProvider
                    template = imageTemplate
                }
            }
        }
        
        // For all watchOS versions:
        if complication.family == .circularSmall {
            let imageTemplate = CLKComplicationTemplateCircularSmallSimpleImage()
            let imageProvider = CLKImageProvider(onePieceImage: UIImage(named: "Complication/Circular")!)
            imageProvider.tintColor = UIColor.blue
            imageTemplate.imageProvider = imageProvider
            template = imageTemplate
        }
        else if complication.family == .modularSmall {
            let imageTemplate = CLKComplicationTemplateModularSmallSimpleImage()
            let imageProvider = CLKImageProvider(onePieceImage: UIImage(named: "Complication/Modular")!)
            imageProvider.tintColor = UIColor.blue
            imageTemplate.imageProvider = imageProvider
            template = imageTemplate
        }
        else if complication.family == .utilitarianSmall {
            let imageTemplate = CLKComplicationTemplateUtilitarianSmallSquare()
            let imageProvider = CLKImageProvider(onePieceImage: UIImage(named: "Complication/Utilitarian")!)
            imageProvider.tintColor = UIColor.blue
            imageTemplate.imageProvider = imageProvider
            template = imageTemplate
        }
        
        return template
    }

    func getSupportedTimeTravelDirections(for complication: CLKComplication, withHandler handler: @escaping (CLKComplicationTimeTravelDirections) -> Void) {
        handler([.forward, .backward])
    }
    
    func getTimelineStartDate(for complication: CLKComplication, withHandler handler: @escaping (Date?) -> Void) {
        handler(nil)
    }
    
    func getTimelineEndDate(for complication: CLKComplication, withHandler handler: @escaping (Date?) -> Void) {
        handler(nil)
    }
    
    func getPrivacyBehavior(for complication: CLKComplication, withHandler handler: @escaping (CLKComplicationPrivacyBehavior) -> Void) {
        handler(.showOnLockScreen)
    }
    
    // MARK: - Timeline Population
    
    func getCurrentTimelineEntry(for complication: CLKComplication, withHandler handler: @escaping (CLKComplicationTimelineEntry?) -> Void) {
        // Call the handler with the current timeline entry
        let template = templateForComplication(complication: complication)
                let timelineEntry = CLKComplicationTimelineEntry(date: Date(), complicationTemplate: template!)
                handler(timelineEntry)
    }
    
    func getTimelineEntries(for complication: CLKComplication, before date: Date, limit: Int, withHandler handler: @escaping ([CLKComplicationTimelineEntry]?) -> Void) {
        // Call the handler with the timeline entries prior to the given date
        handler(nil)
    }
    
    func getTimelineEntries(for complication: CLKComplication, after date: Date, limit: Int, withHandler handler: @escaping ([CLKComplicationTimelineEntry]?) -> Void) {
        // Call the handler with the timeline entries after to the given date
        handler(nil)
    }
    
    func getPlaceholderTemplate(for complication: CLKComplication, withHandler handler: @escaping (CLKComplicationTemplate?) -> Void) {
        // This method will be called once per supported complication, and the results will be cached
        handler(templateForComplication(complication: complication))
    }
    // MARK: - Placeholder Templates
    
    func getLocalizableSampleTemplate(for complication: CLKComplication, withHandler handler: @escaping (CLKComplicationTemplate?) -> Void) {
        // This method will be called once per supported complication, and the results will be cached
        handler(templateForComplication(complication: complication))
    }
    
}
