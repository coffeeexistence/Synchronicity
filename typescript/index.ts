import Rx = require("rxjs");
import createAndReturnVirtualPortStream from "./createAndReturnVirtualPortStream";

const messageTypes = {
  ON: 144,
  OFF: 128
};
const midiPortName = "Light Strip";
const lightCount = 12;

const midiChannelStream = createAndReturnVirtualPortStream(midiPortName);

midiChannelStream.subscribe(message => {
  console.log(message);
});
