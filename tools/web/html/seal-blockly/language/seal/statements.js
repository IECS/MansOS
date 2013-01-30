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
// SEAL: use <id> <param_list> ;
//-------------------------------------
Blockly.Language.seal_use = {
  category: 'Statements',
  helpUrl: 'http://mansos.net/',
  init: function() {
    this.setColour(Seal.block_color);
    this.setPreviousStatement(true);
    this.setNextStatement(true);

    var dropdown = new Blockly.FieldDropdown(function() {
      return Blockly.Language.seal_use.RESOURCES;
    });
    
    this.appendValueInput('ARGS', Blockly.INPUT_VALUE, 'ARGS')
  		.appendTitle('Use')
        .appendTitle(dropdown, 'USE');
      
    this.setTooltip('Use or enable a resource.');
  }
};

Blockly.Language.seal_use.RESOURCES =
    [['RedLed', 'RedLed'],
     ['GreenLed', 'GreenLed'],
     ['BlueLed', 'BlueLed'],
     ['Led', 'Led']];

// SEAL: read <id> <param_list> ;
//-------------------------------------
Blockly.Language.seal_read = {
  category: 'Statements',
  helpUrl: 'http://mansos.net/',
  init: function() {
  	this.setColour(Seal.block_color);
  	this.setPreviousStatement(true);
  	this.setNextStatement(true);

  	var dropdown = new Blockly.FieldDropdown(function () {
  		return Blockly.Language.seal_read.RESOURCES;
  	});

  	this.appendValueInput('ARGS', Blockly.INPUT_VALUE, 'ARGS')
        .appendTitle('read')
        .appendTitle(dropdown, 'READ');

  	this.setTooltip('Read a resource.');
  }
};

Blockly.Language.seal_read.RESOURCES =
    [['ADC', 'ADC'],
     ['Light', 'Light'],
     ['Humidity', 'Humidity'],
     ['Temperature', 'Temperature']];

		 
// SEAL: output <id> <param_list> ;
//-------------------------------------
Blockly.Language.seal_output = {
  category: 'Statements',
  helpUrl: 'http://mansos.net/',
  init: function () {
  	this.setColour(Seal.block_color);
  	this.setPreviousStatement(true);
  	this.setNextStatement(true);

  	var dropdown = new Blockly.FieldDropdown(function () {
  		return Blockly.Language.seal_output.RESOURCES;
  	});

  	this.appendValueInput('ARGS', Blockly.INPUT_VALUE, 'ARGS')
        .appendTitle('output')
        .appendTitle(dropdown, 'OUTPUT');

  	this.setTooltip('Send output.');
  }
};

Blockly.Language.seal_output.RESOURCES =
    [['Radio', 'Radio'],
     ['Serial', 'Serial']];


Blockly.Language.lists_create_with = {
	category: 'Statements',
	helpUrl: 'http://mansos.net/',
	init: function () {
		this.setColour(Seal.block_color);
		this.setPreviousStatement(true);
		this.setNextStatement(true);
		var value = new Blockly.FieldTextInput('Structure name', function (text) {
			Blockly.Language.seal_struct.RESOURCES[0] =
			[String(text), String(text)];
			return String(text);
		});
		this.appendDummyInput()
			.appendTitle('NetworkRead')
			.appendTitle(value, 'NAME');

		this.appendValueInput('ADD0');
		this.setMutator(new Blockly.Mutator(['lists_create_with_item']));
		this.itemCount_ = 1;
	},
	mutationToDom: function (workspace) {
		var container = document.createElement('mutation');
		container.setAttribute('items', this.itemCount_);
		return container;
	},
	domToMutation: function (container) {
		for (var x = 0; x < this.itemCount_; x++) {
			this.removeInput('ADD' + x);
		}
		this.itemCount_ = window.parseInt(container.getAttribute('items'), 10);
		for (var x = 0; x < this.itemCount_; x++) {
			var input = this.appendValueInput('ADD' + x);
			if (x == 0) {
				input.appendTitle("");
			}
		}
		if (this.itemCount_ == 0) {
			this.appendDummyInput('EMPTY')
				.appendTitle("");
		}
	},
	decompose: function (workspace) {
		var containerBlock = new Blockly.Block(workspace,
											   'lists_create_with_container');
		containerBlock.initSvg();
		var connection = containerBlock.getInput('STACK').connection;
		for (var x = 0; x < this.itemCount_; x++) {
			var itemBlock = new Blockly.Block(workspace, 'lists_create_with_item');
			itemBlock.initSvg();
			connection.connect(itemBlock.previousConnection);
			connection = itemBlock.nextConnection;
		}
		return containerBlock;
	},
	compose: function (containerBlock) {
		// Disconnect all input blocks and remove all inputs.
		if (this.itemCount_ == 0) {
			this.removeInput('EMPTY');
		} else {
			for (var x = this.itemCount_ - 1; x >= 0; x--) {
				this.removeInput('ADD' + x);
			}
		}
		this.itemCount_ = 0;
		// Rebuild the block's inputs.
		var itemBlock = containerBlock.getInputTargetBlock('STACK');
		while (itemBlock) {
			var input = this.appendValueInput('ADD' + this.itemCount_);
			if (this.itemCount_ == 0) {
				input.appendTitle("");
			}
			// Reconnect any child blocks.
			if (itemBlock.valueConnection_) {
				input.connection.connect(itemBlock.valueConnection_);
			}
			this.itemCount_++;
			itemBlock = itemBlock.nextConnection &&
				itemBlock.nextConnection.targetBlock();
		}
		if (this.itemCount_ == 0) {
			this.appendDummyInput('EMPTY')
				.appendTitle("");
		}
	},
	saveConnections: function (containerBlock) {
		// Store a pointer to any connected child blocks.
		var itemBlock = containerBlock.getInputTargetBlock('STACK');
		var x = 0;
		while (itemBlock) {
			var input = this.getInput('ADD' + x);
			itemBlock.valueConnection_ = input && input.connection.targetConnection;
			x++;
			itemBlock = itemBlock.nextConnection &&
				itemBlock.nextConnection.targetBlock();
		}
	}
};
  
Blockly.Language.lists_create_with_container = {
	// Container.
	init: function () {
		this.setColour(210);
		this.appendDummyInput()
			.appendTitle(Blockly.LANG_LISTS_CREATE_WITH_CONTAINER_TITLE_ADD);
		this.appendStatementInput('STACK');
		this.setTooltip(Blockly.LANG_LISTS_CREATE_WITH_CONTAINER_TOOLTIP_1);
		this.contextMenu = false;
	}
};

Blockly.Language.lists_create_with_item = {
	// Add items.
	init: function () {
		this.setColour(210);
		this.appendDummyInput()
			.appendTitle(Blockly.LANG_LISTS_CREATE_WITH_ITEM_TITLE);
		this.setPreviousStatement(true);
		this.setNextStatement(true);
		this.setTooltip(Blockly.LANG_LISTS_CREATE_WITH_ITEM_TOOLTIP_1);
		this.contextMenu = false;
	}
};