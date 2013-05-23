/**
 * Visual Blocks Language
 *
 * Copyright 2012 Google Inc.
 * http://code.google.com/p/google-blockly/
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * @fileoverview SEAL blocks for Blockly.
 * @author leo@selavo.com	(Leo Selavo)
 */

if (!Blockly.Language) {
  Blockly.Language = {};
}

/**
 * Create a namespace for the Seal.
 */
var Seal = {};
Seal.block_color = 15;

Blockly.Seal = Blockly.Generator.get('Seal');

Blockly.Language.seal_period = {
  category: 'Parameters',
  helpUrl: 'http://open-sci.net/',
  init: function () {
  	this.setColour(Seal.block_color);
  	this.setOutput(true);

    var dropdown = new Blockly.FieldDropdown(function() {
      return Blockly.Language.seal_period.TIME;
    });
    dropdown.setValue("s");
    var value = new Blockly.FieldTextInput('1', function(text) {
      var n = window.parseFloat(text || 0);
      return window.isNaN(n) ? null : String(n);
    });

    this.appendValueInput('ARGS', Blockly.INPUT_VALUE, 'ARGS')
        .appendTitle('period')
        .appendTitle(value, 'NUM')
  		.appendTitle(dropdown, 'PERIOD');
  }
};

Blockly.Language.seal_period.TIME =
    [['ms', 'ms'],
     ['s', 's']];
 
Blockly.Language.seal_state = {
	// Boolean data type: true and false.
	category: "Parameters",
	helpUrl: 'http://open-sci.net/',
	init: function () {
		this.setColour(Seal.block_color);
		this.setOutput(true, String);
		this.appendValueInput('ARGS', Blockly.INPUT_VALUE, 'ARGS')
			.appendTitle(new Blockly.FieldDropdown(this.OPERATORS), 'BOOL');
		this.setTooltip(Blockly.LANG_LOGIC_BOOLEAN_TOOLTIP_1);
	}
};

Blockly.Language.seal_state.OPERATORS =
    [['On', 'On'],
     ['Off', 'Off']];