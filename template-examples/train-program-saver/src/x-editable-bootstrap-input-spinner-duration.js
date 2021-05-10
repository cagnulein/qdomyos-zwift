(function ($) {
    'use strict';
    
    var DurationInputSpinner = function (options) {
        this.init('durationinputspinner', options, DurationInputSpinner.defaults);
    };

    //inherit from Abstract input
    $.fn.editableutils.inherit(DurationInputSpinner, $.fn.editabletypes.abstractinput);

    $.extend(DurationInputSpinner.prototype, {
        /**
        Renders input from tpl

        @method render() 
        **/        
        render: function() {
            this.$input = this.$tpl.find('input');
            this.$input.closest('.editable-input').removeClass('editable-input').addClass('editable-input-spinner');
            this.$input.inputSpinner({editor: DurationEditor});
        },
        
        /**
        Default method to show value in element. Can be overwritten by display option.
        
        @method value2html(value, element) 
        **/
        value2html: function(value, element) {
            if(value === undefined || value === null) {
                $(element).empty();
                return; 
            }
            $(element).html(training_render_duration_hms(training_parse_duration(value), 1)); 
        },
        
        /**
        Gets value from element's html
        
        @method html2value(html) 
        **/        
        html2value: function(html) {        
            /*
            you may write parsing method to get value by element's html
            e.g. "Moscow, st. Lenina, bld. 15" => {city: "Moscow", street: "Lenina", building: "15"}
            but for complex structures it's not recommended.
            Better set value directly via javascript, e.g. 
            editable({
                value: {
                    city: "Moscow", 
                    street: "Lenina", 
                    building: "15"
                }
            });
          */ 
            return null;  
        },
      
        /**
        Converts value to string. 
        It is used in internal comparing (not for sending to server).
        
        @method value2str(value)  
       **/
        value2str: function(value) {
            return value + '';
        }, 
       
        /*
        Converts string to value. Used for reading value from 'data-value' attribute.
        
        @method str2value(str)  
       */
        str2value: function(str) {
            /*
           this is mainly for parsing value defined in data-value attribute. 
           If you will always set value by javascript, no need to overwrite it
           */
            return str;
        },                
       
        /**
        Sets value of input.
        
        @method value2input(value) 
        @param {mixed} value
       **/         
        value2input: function(value) {
            if(!value) {
                return;
            }
            this.$input.val(training_parse_duration(value));
        },       
       
        /**
        Returns value of input.
        
        @method input2value() 
       **/          
        input2value: function() { 
            return training_render_duration_hms(this.$input.val());
        },        
       
        /**
        Activates input: sets focus on the first field.
        
        @method activate() 
       **/        
        activate: function() {
            this.$input.focus();
        },  
       
        /**
        Attaches handler to submit form in case of 'showbuttons=false' mode
        
        @method autosubmit() 
       **/       
        autosubmit: function() {
            this.$input.keydown(function (e) {
                if (e.which === 13) {
                    $(this).closest('form').submit();
                }
            });
        }       
    });

    DurationInputSpinner.defaults = $.extend({}, $.fn.editabletypes.abstractinput.defaults, {
        tpl: '<span><input type="number" value="1" step="1" required/></span>',
             
        inputclass: ''
    });

    $.fn.editabletypes.durationinputspinner = DurationInputSpinner;

}(window.jQuery));