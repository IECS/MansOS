'use strict';

Blockly.Seal = Blockly.Generator.get('Seal');

Blockly.Seal.seal_period = function () {
	return [', period ' + this.getTitleValue('NUM').toLowerCase() + this.getTitleValue('PERIOD') + Blockly.Seal.valueToCode(this, 'ARGS')];
};

Blockly.Seal.seal_state = function () {
	return [', ' + this.getTitleValue('BOOL').toLowerCase() +  Blockly.Seal.valueToCode(this, 'ARGS')];
};