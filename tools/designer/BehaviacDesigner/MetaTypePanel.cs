/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Tencent is pleased to support the open source community by making behaviac available.
//
// Copyright (C) 2015 THL A29 Limited, a Tencent company. All rights reserved.
//
// Licensed under the BSD 3-Clause License (the "License"); you may not use this file except in compliance with
// the License. You may obtain a copy of the License at http://opensource.org/licenses/BSD-3-Clause
//
// Unless required by applicable law or agreed to in writing, software distributed under the License is
// distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and limitations under the License.
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Text;
using System.Windows.Forms;
using System.Reflection;
using Behaviac.Design.Attributes;
using Behaviac.Design.Data;
using Behaviac.Design.Network;
using Behaviac.Design.Nodes;
using Behaviac.Design.Properties;

namespace Behaviac.Design
{
    internal partial class MetaTypePanel : UserControl
    {
        public enum MetaTypes
        {
            Agent,
            Enum,
            Struct
        }

        private bool _initialized = false;
        private ToolTip _toolTip = new ToolTip();

        public MetaTypePanel()
        {
            InitializeComponent();

            _toolTip.AutoPopDelay = 5000;
            _toolTip.InitialDelay = 500;
            _toolTip.ReshowDelay = 5000;
            _toolTip.ShowAlways = true;

            _toolTip.SetToolTip(this.extendCheckBox, Resources.IsTypeExtented);
        }

        public MetaTypes GetMetaType()
        {
            return (MetaTypes)this.typeComboBox.SelectedIndex;
        }

        private bool _isModified = false;
        public bool IsModified
        {
            get { return _isModified; }
            set { _isModified = true; }
        }

        public string GetNamespace()
        {
            return this.namespaceTextBox.Text;
        }

        private AgentType _customizedAgent = null;
        public AgentType GetCustomizedAgent()
        {
            if (this.GetMetaType() == MetaTypes.Agent)
            {
                if (this.baseComboBox.SelectedIndex > -1)
                {
                    Debug.Check(_customizedAgent != null);

                    AgentType baseAgent = Plugin.AgentTypes[this.baseComboBox.SelectedIndex];
                    string agentName = this.nameTextBox.Text;
                    if (!string.IsNullOrEmpty(this.namespaceTextBox.Text))
                    {
                        agentName = this.namespaceTextBox.Text + "::" + agentName;
                    }

                    _customizedAgent.Reset(this.extendCheckBox.Checked, agentName, baseAgent, this.dispTextBox.Text, this.descTextBox.Text);
                }
            }

            return _customizedAgent;
        }

        private EnumType _enumType = null;
        public EnumType GetCustomizedEnum()
        {
            if (this.GetMetaType() == MetaTypes.Enum)
            {
                Debug.Check(_enumType != null);

                _enumType.Reset(true, this.extendCheckBox.Checked, this.nameTextBox.Text, this.namespaceTextBox.Text, this.dispTextBox.Text, this.descTextBox.Text);

            }

            return _enumType;
        }

        private StructType _structType = null;
        public StructType GetCustomizedStruct()
        {
            if (this.GetMetaType() == MetaTypes.Struct)
            {
                Debug.Check(_structType != null);

                _structType.Reset(true, this.extendCheckBox.Checked, this.nameTextBox.Text, this.namespaceTextBox.Text, this.baseComboBox.Text, this.dispTextBox.Text, this.descTextBox.Text);
            }

            return _structType;
        }

        private bool _isNew = false;

        public void Initialize(object typeObject)
        {
            _initialized = false;
            _isModified = false;
            _isNew = (typeObject == null);
            this.Text = _isNew ? Resources.AddType : Resources.EditType;

            MetaTypes metaType = MetaTypes.Agent;

            if (typeObject != null)
            {
                if (typeObject is AgentType)
                {
                    metaType = MetaTypes.Agent;
                    _customizedAgent = typeObject as AgentType;
                }
                else if (typeObject is EnumType)
                {
                    metaType = MetaTypes.Enum;
                    _enumType = typeObject as EnumType;
                }
                else if (typeObject is StructType)
                {
                    metaType = MetaTypes.Struct;
                    _structType = typeObject as StructType;
                }
            }

            this.typeComboBox.Items.Clear();
            this.typeComboBox.Enabled = _isNew;
            foreach (string type in Enum.GetNames(typeof(MetaTypes)))
            {
                this.typeComboBox.Items.Add(type);
            }
            this.typeComboBox.SelectedIndex = (int)metaType;

            if (this.GetMetaType() == MetaTypes.Agent)
            {
                Debug.Check(_customizedAgent != null);

                if (_customizedAgent.Base != null)
                {
                    this.baseComboBox.SelectedText = _customizedAgent.Base.AgentTypeName;
                }

                this.extendCheckBox.Checked = _customizedAgent.IsExtended;
                this.nameTextBox.Text = _customizedAgent.BasicClassName;
                this.namespaceTextBox.Text = _customizedAgent.Namespace;
                this.dispTextBox.Text = _customizedAgent.DisplayName;
                this.descTextBox.Text = _customizedAgent.Description;

                this.extendCheckBox.Enabled = _customizedAgent.IsCustomized;
                this.nameTextBox.Enabled = _customizedAgent.IsCustomized;
                this.namespaceTextBox.Enabled = _customizedAgent.IsCustomized;
                this.baseComboBox.Enabled = _customizedAgent.IsCustomized;
                this.dispTextBox.Enabled = _customizedAgent.IsCustomized;
                this.descTextBox.Enabled = _customizedAgent.IsCustomized;
            }
            else
            {
                if (this.GetMetaType() == MetaTypes.Enum)
                {
                    Debug.Check(_enumType != null);

                    this.extendCheckBox.Checked = _enumType.IsExtended;
                    this.nameTextBox.Text = _enumType.Name;
                    this.namespaceTextBox.Text = _enumType.Namespace;
                    this.dispTextBox.Text = _enumType.DisplayName;
                    this.descTextBox.Text = _enumType.Description;

                    this.baseComboBox.Visible = false;

                    this.extendCheckBox.Enabled = _enumType.IsCustomized;
                    this.nameTextBox.Enabled = _enumType.IsCustomized;
                    this.baseComboBox.Enabled = _enumType.IsCustomized;
                    this.namespaceTextBox.Enabled = _enumType.IsCustomized;
                    this.baseComboBox.Enabled = _enumType.IsCustomized;
                    this.dispTextBox.Enabled = _enumType.IsCustomized;
                    this.descTextBox.Enabled = _enumType.IsCustomized;
                }
                else if (this.GetMetaType() == MetaTypes.Struct)
                {
                    Debug.Check(_structType != null);

                    this.extendCheckBox.Checked = _structType.IsExtended;
                    this.nameTextBox.Text = _structType.Name;
                    this.namespaceTextBox.Text = _structType.Namespace;
                    this.baseComboBox.SelectedText = _structType.BaseName;
                    this.dispTextBox.Text = _structType.DisplayName;
                    this.descTextBox.Text = _structType.Description;

                    this.extendCheckBox.Enabled = _structType.IsCustomized;
                    this.nameTextBox.Enabled = _structType.IsCustomized;
                    this.baseComboBox.Enabled = _structType.IsCustomized;
                    this.namespaceTextBox.Enabled = _structType.IsCustomized;
                    this.baseComboBox.Enabled = _structType.IsCustomized;
                    this.dispTextBox.Enabled = _structType.IsCustomized;
                    this.descTextBox.Enabled = _structType.IsCustomized;
                }
            }

            if (this.extendCheckBox.Checked)
            {
                this.baseComboBox.Visible = false;
                this.baseLabel.Visible = false;
                if (this.baseComboBox.Items.Count > 0)
                    this.baseComboBox.SelectedIndex = 0;
            }
            else
            {
                this.baseComboBox.Visible = true;
                this.baseLabel.Visible = true;
            }

            if (_isNew && string.IsNullOrEmpty(this.namespaceTextBox.Text))
            {
                this.namespaceTextBox.Text = Settings.Default.DefaultNamespace;
            }

            this.nameTextBox.Focus();
            if (this.nameTextBox.TextLength > 0)
            {
                this.nameTextBox.SelectionStart = this.nameTextBox.TextLength;
            }
            else
            {
                this.nameTextBox.Select();
            }

            _initialized = true;
        }

        private void resetBaseTypes()
        {
            this.baseComboBox.Items.Clear();
            this.baseComboBox.Visible = false;
            this.baseLabel.Visible = false;

            if (this.GetMetaType() == MetaTypes.Agent || this.GetMetaType() == MetaTypes.Struct)
            {
                this.baseComboBox.Visible = true;
                this.baseLabel.Visible = true;

                int baseIndex = -1;

                if (this.GetMetaType() == MetaTypes.Agent)
                {
                    for (int i = 0; i < Plugin.AgentTypes.Count; ++i)
                    {
                        AgentType agentType = Plugin.AgentTypes[i];
                        this.baseComboBox.Items.Add(agentType.AgentTypeName);

                        if (this._customizedAgent != null && this._customizedAgent.Base != null && this._customizedAgent.Base.AgentTypeName == agentType.AgentTypeName)
                        {
                            baseIndex = i;
                        }
                    }

                    if (baseIndex < 0 && Plugin.AgentTypes.Count > 0)
                    {
                        baseIndex = 0;
                    }
                }
                else // struct
                {
                    this.baseComboBox.Items.Add("");
                    for (int i = 0; i < TypeManager.Instance.Structs.Count; ++i)
                    {
                        StructType structType = TypeManager.Instance.Structs[i];

                        if (this._structType == null ||
                            string.IsNullOrEmpty(this._structType.Name) ||
                            this._structType.Name != structType.Name &&
                            this._structType.Name != structType.BaseName)
                        {
                            this.baseComboBox.Items.Add(structType.Name);

                            if (this._structType != null && this._structType.BaseName == structType.Name)
                            {
                                baseIndex = i;
                            }
                        }
                    }
                }

                if (this.extendCheckBox.Checked)
                {
                    if (this.baseComboBox.Items.Count > 0)
                        this.baseComboBox.SelectedIndex = 0;
                }
                else
                {
                    this.baseComboBox.SelectedIndex = baseIndex;
                }
            }
        }

        public bool Verify()
        {
            if (string.IsNullOrEmpty(this.nameTextBox.Text) || this.nameTextBox.Text.Length < 1 || !char.IsLetter(this.nameTextBox.Text[0]))
                return false;

            foreach (AgentType agent in Plugin.AgentTypes)
            {
                if (this.GetMetaType() == MetaTypes.Agent && _customizedAgent == agent)
                    return true;

                if (agent.AgentTypeName == this.nameTextBox.Text)
                    return false;
            }

            foreach (EnumType customizedEnum in TypeManager.Instance.Enums)
            {
                if (this.GetMetaType() == MetaTypes.Enum && _enumType == customizedEnum)
                    return true;

                if (customizedEnum.Name == this.nameTextBox.Text)
                    return false;
            }

            foreach (StructType structType in TypeManager.Instance.Structs)
            {
                if (this.GetMetaType() == MetaTypes.Struct && _structType == structType)
                    return true;

                if (structType.Name == this.nameTextBox.Text)
                    return false;
            }

            return this.GetMetaType() != MetaTypes.Agent || this.baseComboBox.SelectedIndex > -1;
        }

        private void typeComboBox_SelectedIndexChanged(object sender, EventArgs e)
        {
            if (this.GetMetaType() == MetaTypes.Agent)
            {
                if (_customizedAgent == null)
                {
                    string agentName = this.nameTextBox.Text;
                    if (!string.IsNullOrEmpty(this.namespaceTextBox.Text))
                    {
                        agentName = this.namespaceTextBox.Text + "::" + agentName;
                    }

                    _customizedAgent = new AgentType(this.extendCheckBox.Checked, agentName, null, this.dispTextBox.Text, this.descTextBox.Text);
                }
            }
            else if (this.GetMetaType() == MetaTypes.Enum)
            {
                if (_enumType == null)
                {
                    _enumType = new EnumType(true, this.extendCheckBox.Checked, this.nameTextBox.Text, this.namespaceTextBox.Text, this.dispTextBox.Text, this.descTextBox.Text);
                }
            }
            else if (this.GetMetaType() == MetaTypes.Struct)
            {
                if (_structType == null)
                {
                    _structType = new StructType(true, this.extendCheckBox.Checked, this.nameTextBox.Text, this.namespaceTextBox.Text, this.baseComboBox.Text, this.dispTextBox.Text, this.descTextBox.Text);
                }
            }

            resetBaseTypes();

            if (_initialized)
                this.IsModified = true;
        }

        private void extendCheckBox_CheckedChanged(object sender, EventArgs e)
        {
            if (_initialized)
            {
                this.IsModified = true;

                if (this.extendCheckBox.Checked)
                {
                    this.baseComboBox.Visible = false;
                    this.baseLabel.Visible = false;
                    if (this.baseComboBox.Items.Count > 0)
                        this.baseComboBox.SelectedIndex = 0;
                }
                else
                {
                    this.baseComboBox.Visible = true;
                    this.baseLabel.Visible = true;
                }
            }
        }

        private void nameTextBox_TextChanged(object sender, EventArgs e)
        {
            if (_initialized)
                this.IsModified = true;
        }

        private void namespaceTextBox_TextChanged(object sender, EventArgs e)
        {
            if (_initialized)
                this.IsModified = true;
        }

        private void baseComboBox_SelectedIndexChanged(object sender, EventArgs e)
        {
            if (_initialized)
                this.IsModified = true;
        }

        private void dispTextBox_TextChanged(object sender, EventArgs e)
        {
            if (_initialized)
                this.IsModified = true;
        }

        private void descTextBox_TextChanged(object sender, EventArgs e)
        {
            if (_initialized)
                this.IsModified = true;
        }
    }
}
