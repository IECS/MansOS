if (!Blockly.Language) {
	Blockly.Language = {};
}

/**
 * Create a namespace for the Seal.
 */
var Seal = {};
Seal.block_color = 15;

Blockly.Seal = Blockly.Generator.get('Seal');

Blockly.Language.seal_number = {
	category: "Values",
	helpUrl: 'http://mansos.net/',
	init: function () {
		this.setColour(230);
		this.appendDummyInput()
			.appendTitle(new Blockly.FieldTextInput('0', null), 'NUM');
		this.setOutput(true, Number);
		this.setTooltip(Blockly.LANG_MATH_NUMBER_TOOLTIP_1);
	}
};

Blockly.Language.seal_sensors = {
	category: "Values",
	helpUrl: 'http://mansos.net/',
	init: function () {
		this.setColour(230);
		this.setOutput(true, String);
		this.appendDummyInput()
			.appendTitle(new Blockly.FieldDropdown(Blockly.Language.seal_read.RESOURCES), 'VALUE');
		this.setTooltip(Blockly.LANG_LOGIC_BOOLEAN_TOOLTIP_1);
	}
};


Blockly.Language.seal_struct = {
	category: 'Values',
	helpUrl: 'http://mansos.net/',
	init: function () {
		this.setColour(230);
		this.setOutput(true, String);

		//var value = new Blockly.FieldTextInput('Structure name', function (text) {
		//	return String(text);
		//});
		var dropdown = new Blockly.FieldDropdown(function () {
			return Blockly.Language.seal_struct.RESOURCES;
		});

		this.appendValueInput('ARGS', Blockly.INPUT_VALUE, 'ARGS')
			.appendTitle(dropdown, 'NAME')
			.appendTitle('.');
	}
};

Blockly.Language.seal_struct.RESOURCES =
	[["No struct found", "NSF"]];
