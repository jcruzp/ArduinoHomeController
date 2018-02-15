// Alexa SDK for Arduino Home Controller
// Copyright (c) 2014-2015 Amazon.com, Inc. or its affiliates. All Rights Reserved. Use is subject to license terms.
// Jose Cruz https://sites.google.com/view/jriot 

// Define connection to PubNub 
var PubNub = require("pubnub");
var pubnub = new PubNub({
    ssl: true,
    publish_key: "pub-c-e93def7f-95aa-475a-aa60-cc3cd32ee8a7",
    subscribe_key: "sub-c-ec04dbbc-0893-11e8-8e75-cea83f8405bb",
    uuid: "YOUR UUID"
});

// Define the PubNub channel
var channel = 'AHC_IOT_01';
// Used to receive some values (temperature, humidity)
var channel2 = 'AHC_IOT_02';

// Define slot states for light and alarm turn on/off
var slotStates = ['on', 'off'];

// App ID for the skill
var APP_ID = 'YOUR APP ID FOR THE SKILL';

// The AlexaSkill prototype and helper functions
var AlexaSkill = require('./AlexaSkill');

var ArduinoHomeSkill = function() {
    AlexaSkill.call(this, APP_ID);
};

//Listener for suscribe and receive temperature and humidity
var mqttListener;

var access_token;
var url;
var https = require("https");


function SendMessage(topicname, commandvalue, speechOutput, iresponse) {
    https.get(url, res => {
        res.setEncoding("utf8");
        let body = "";
        res.on("data", data => {
            body += data;
        });
        res.on("end", () => {
            pubnub.publish({ //Publishes the turn message to my PubHub Device.
                channel: channel,
                message: {
                    "topic": topicname,
                    "command": commandvalue,
                    "id": JSON.parse(body).email
                }
            }).then((response) => {
                console.log("message Published w/ timetoken", response.timetoken);
                if (speechOutput) iresponse.tell(speechOutput);
            }).catch((error) => {
                console.log("publishing failed w/ status: ", error);
                iresponse.ask("Sorry, I didn't catch what you said");
            });

        });
    });
}


/// Extend AlexaSkill
ArduinoHomeSkill.prototype = Object.create(AlexaSkill.prototype);
ArduinoHomeSkill.prototype.constructor = ArduinoHomeSkill;

ArduinoHomeSkill.prototype.eventHandlers.onSessionStarted = function(sessionStartedRequest, session) {

    //    SetUserID();

    console.log("ArduinoHomeSkill onSessionStarted requestId: " + sessionStartedRequest.requestId +
        ", sessionId: " + session.sessionId);
    //    console.log("Init...");

    //Delete all listeners and suscribes defined 
    pubnub.removeListener(mqttListener);
    pubnub.unsubscribeAll();

    //    console.log("Init...Ok");

};

//-------->This is invoked by invocation word "Arduino Home"
ArduinoHomeSkill.prototype.eventHandlers.onLaunch = function(launchRequest, session, response) {
    console.log("ArduinoHomeSkill onLaunch requestId: " + launchRequest.requestId + ", sessionId: " + session.sessionId);
    //if no amazon token, return a LinkAccount card

    if (session.user.accessToken == undefined) {

        response.tellWithCardLink('To start using this skill, please use the companion app to authenticate on Amazon.');

        return;

    }

    var speechOutput = "Welcome to Arduino Home Controller.   What would you like to do?";
    var repromptText = "I am ready";
    response.ask(speechOutput, repromptText);
};

ArduinoHomeSkill.prototype.eventHandlers.onSessionEnded = function(sessionEndedRequest, session) {
    console.log("ArduinoHomeSkill onSessionEnded requestId: " + sessionEndedRequest.requestId +
        ", sessionId: " + session.sessionId);

    console.log("End...");

};



//*** Define all intent handlers
ArduinoHomeSkill.prototype.intentHandlers = {
    //*** AboutIntent handler
    "AboutIntent": function(intent, session, response) {
        var myText;
        console.log("in about");
        myText = "Arduino Home Controller skil let you control internet connected devices. It controls  Lights at room, kitchen, garage or living room), read a temperature sensor to scan home temperature, read Humidity sensor to scan home humidity, uses webcam to take a home security photo and sent it by email and activate an alarm to alert some events. It uses PubNub site to manipulate all messages send by Alexa with Lambda function. Please check information at skill page for more details.   What would you like to do?";
        response.ask(myText);
        return;
    },

    //*** LightIntent handler
    "LightIntent": function(intent, session, response) {
        var slotHabRooms = ['room', 'kitchen', 'garage', 'living room'];

        var lightSlot = intent.slots.light;
        var lightSlotValue = lightSlot ? lightSlot.value : "";

        var whichSlot = intent.slots.which;
        var whichSlotValue = whichSlot ? whichSlot.value : "";
        if (lightSlotValue && whichSlotValue && slotStates.indexOf(lightSlotValue.toLowerCase()) > -1 && slotHabRooms.indexOf(whichSlotValue.toLowerCase()) > -1) {

            SendMessage('Topic_' + whichSlotValue, lightSlotValue, "The light is now " + lightSlotValue, response);

        }
        else {
            response.ask("Sorry, I didn't catch what you said");
        }
    },

    //*** AlarmIntent handler
    "AlarmIntent": function(intent, session, response) {
        var alarmSlot = intent.slots.alarm;
        var alarmSlotValue = alarmSlot ? alarmSlot.value : "";
        if (alarmSlotValue && slotStates.indexOf(alarmSlotValue.toLowerCase()) > -1) {
            SendMessage('Topic_alarm', alarmSlotValue, "The alarm is now " + alarmSlotValue, response);
        }
        else {
            response.ask("Sorry, I didn't catch what you said");
        }
    },

    //*** TakePhotoIntent handler
    "TakePhotoIntent": function(intent, session, response) {

        SendMessage('Topic_photo', 'take', "Taken home photo ", response);

    },
    //*** GetTemperatureIntent handler
    "GetTemperatureIntent": function(intent, session, response) {

        mqttListener = {
            status: function(statusEvent) {},
            message: function(message) {
                // handle message
                console.log("Receive=", message);
                var myText = "Inside Temperature is " + message.message + " degrees C";
                response.tell(myText);
            },
            presence: function(presenceEvent) {
                // handle presence
            }
        };
        pubnub.addListener(mqttListener);
        pubnub.subscribe({
            channels: [channel2]
        });
        SendMessage('Topic_temperature', 'Ok', null, response)
    },


    //*** GetTemperatureIntent handler
    "GetHumidityIntent": function(intent, session, response) {
        mqttListener = {
            status: function(statusEvent) {},
            message: function(message) {
                // handle message
                console.log("Receive=", message);
                var myText = "Inside Humidity is " + message.message + " %";
                response.tell(myText);
            },
            presence: function(presenceEvent) {
                // handle presence
            }
        };

        pubnub.addListener(mqttListener);

        pubnub.subscribe({
            channels: [channel2]
        });

        SendMessage('Topic_humidity', 'Ok', null, response)
    },

    //*** HelpIntent handler
    "AMAZON.HelpIntent": function(intent, session, response) {
        response.ask("With Arduino Home Controller skill and Alexa you can control internet connected devices using an Arduino Yun or Arduino with Ethernet Shield. Please check information at skill page for more details.    What would you like to do?");
    },
    //*** StopIntent handler
    "AMAZON.StopIntent": function(intent, session, response) {
        response.tell("Thanks for using Arduino Home Controller. Bye see you later");
    },
    //*** StopIntent handler
    "AMAZON.CancelIntent": function(intent, session, response) {
        response.tell("Thanks for using Arduino Home Controller. Bye see you later");
    },
    default: function(intent, session, response) {
        response.ask("Try again");
    },

};


// Create the handler that responds to the Alexa Request.
exports.handler = function(event, context) {

    try {
        access_token = event['context']['System']['user']['accessToken'];
        url = 'https://api.amazon.com/user/profile?access_token=' + access_token;
        //console.log("Access Token:", access_token);
    }
    catch (error) {
        console.log(error);
    }

    // Create an instance of Arduino Home Skill
    var ArduinoHomeControl = new ArduinoHomeSkill();
    ArduinoHomeControl.execute(event, context);
    //console.log('AWSrequestID =', context.awsRequestId);
};

