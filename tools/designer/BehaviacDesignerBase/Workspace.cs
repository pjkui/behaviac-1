////////////////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009, Daniel Kollmann
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification, are permitted
// provided that the following conditions are met:
//
// - Redistributions of source code must retain the above copyright notice, this list of conditions
//   and the following disclaimer.
//
// - Redistributions in binary form must reproduce the above copyright notice, this list of
//   conditions and the following disclaimer in the documentation and/or other materials provided
//   with the distribution.
//
// - Neither the name of Daniel Kollmann nor the names of its contributors may be used to endorse
//   or promote products derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
// FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY
// WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// The above software in this distribution may have been modified by THL A29 Limited ("Tencent Modifications").
//
// All Tencent Modifications are Copyright (C) 2015 THL A29 Limited.
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using System;
using System.Collections.Generic;
using System.Text;
using System.IO;
using System.Windows.Forms;
using System.Xml;
using Behaviac.Design.Properties;
using Behaviac.Design.Attributes;
using Behaviac.Design.Exporters;

namespace Behaviac.Design
{
    /// <summary>
    /// This class represents a workspace available to the user.
    /// </summary>
    public sealed class Workspace
    {
        public delegate void WorkspaceChangedDelegate();
        public static WorkspaceChangedDelegate WorkspaceChangedHandler;

        private static Workspace _current = null;
        public static Workspace Current
        {
            get { return _current; }
            set
            {
                if (_current != value)
                {
                    _current = value;

                    if (WorkspaceChangedHandler != null)
                    {
                        WorkspaceChangedHandler();
                    }
                }
            }
        }

        public static bool DebugWorkspace = true;

        private string _name = "";
        public string Name
        {
            get { return _name; }
        }

        private string _language = "cpp";
        public string Language
        {
            get { return _language; }
            set
            {
                if (!string.IsNullOrEmpty(value))
                {
                    _language = value;

                    Debug.Check(value == "cpp" || value == "cs", "Only cpp or cs are supported now!");
                }
            }
        }

        private int _version = 3;
        public int Version
        {
            get { return _version; }
            set { _version = value; }
        }

        private string _file_name;
        public string FileName
        {
            get
            {
                //return Path.Combine(_folder, _name) + ".xml";
                return _file_name;
            }
        }

        private string _folder;

        /// <summary>
        /// The folder the behaviors are saved in.
        /// </summary>
        public string Folder
        {
            get
            {
                _folder = Path.GetFullPath(_folder);
                return _folder;
            }
        }

        public string RelativeFolder
        {
            get { return MakeRelativePath(this.Folder); }
        }

        private string _defaultExportFolder;

        /// <summary>
        /// The default folder behaviours are exported to.
        /// </summary>
        public string DefaultExportFolder
        {
            get { return _defaultExportFolder; }
        }

        public string RelativeDefaultExportFolder
        {
            get { return MakeRelativePath(_defaultExportFolder); }
        }

        private string _metaFilename;
        public string MetaFilename
        {
            get { return _metaFilename; }
            set { _metaFilename = value; }
        }

        public string RelativeMetaFilename
        {
            get { return string.IsNullOrEmpty(_metaFilename) ? "" : MakeRelativePath(_metaFilename); }
        }

        public class ExportData
        {
            private bool _isExported = true;
            public bool IsExported
            {
                get { return _isExported; }
                set { _isExported = value; }
            }

            private int _exportFileCount = 1;
            public int ExportFileCount
            {
                get { return _exportFileCount; }
                set { _exportFileCount = value; }
            }

            /// <summary>
            /// ExportFolder should be saved as relative path, but used as absoluted path.
            /// </summary>
            private string _exportFolder = "";
            public string ExportFolder
            {
                get { return _exportFolder; }
                set { _exportFolder = value; }
            }

            /// <summary>
            /// ExportIncludedFilenames should be saved and used as relative path.
            /// </summary>
            private List<string> _exportIncludedFilenames = new List<string>();
            public List<string> ExportIncludedFilenames
            {
                get { return _exportIncludedFilenames; }
            }
        }

        private Dictionary<string, ExportData> _exportDatas = new Dictionary<string, ExportData>();
        public Dictionary<string, ExportData> ExportDatas
        {
            get { return _exportDatas; }
        }

        public void SetExportInfo(string format, bool isExported, int exportFileCount, string folder = null, List<string> includedFilenames = null)
        {
            if (string.IsNullOrEmpty(format))
            {
                return;
            }

            if (!_exportDatas.ContainsKey(format))
            {
                _exportDatas[format] = new ExportData();
            }

            ExportData data = _exportDatas[format];
            data.IsExported = isExported;
            data.ExportFileCount = exportFileCount;

            if (folder != null)
            {
                data.ExportFolder = folder;
            }

            if (includedFilenames != null)
            {
                data.ExportIncludedFilenames.Clear();
                data.ExportIncludedFilenames.AddRange(includedFilenames);
            }
        }

        public void SetIncludedFilenames(string format, List<string> includedFilenames)
        {
            if (string.IsNullOrEmpty(format))
            {
                return;
            }

            if (includedFilenames != null)
            {
                if (!_exportDatas.ContainsKey(format))
                {
                    _exportDatas[format] = new ExportData();
                }

                ExportData data = _exportDatas[format];
                data.ExportIncludedFilenames.Clear();
                data.ExportIncludedFilenames.AddRange(includedFilenames);
            }
        }

        public bool ShouldBeExported(string format)
        {
            if (_exportDatas.ContainsKey(format))
            {
                ExportData data = _exportDatas[format];
                return data.IsExported;
            }

            // export xml only by default
            if (format == "xml")
            {
                return true;
            }

            return false;
        }

        public int GetExportFileCount(string format)
        {
            if (format == "xml" || format == "bson")
            {
                return 1;
            }

            if (_exportDatas.ContainsKey(format))
            {
                ExportData data = _exportDatas[format];
                return data.ExportFileCount;
            }

            return 1;
        }

        public void SetExportFileCount(string format, int fileCount)
        {
            if (format == "xml" || format == "bson")
            {
                return;
            }

            if (!_exportDatas.ContainsKey(format))
            {
                _exportDatas[format] = new ExportData();
            }

            ExportData data = _exportDatas[format];
            data.ExportFileCount = fileCount;
        }

        public bool IsSetExportFolder(string format)
        {
            if (_exportDatas.ContainsKey(format))
            {
                ExportData data = _exportDatas[format];
                return !string.IsNullOrEmpty(data.ExportFolder);
            }

            return false;
        }

        public string GetExportFolder(string format)
        {
            string exportFolder = "";

            if (_exportDatas.ContainsKey(format))
            {
                ExportData data = _exportDatas[format];
                exportFolder = data.ExportFolder;
            }

            if (string.IsNullOrEmpty(exportFolder))
            {
                string wsFilename = this.FileName.Replace('/', '\\');
                exportFolder = this.DefaultExportFolder.Replace('/', '\\');
                exportFolder = Workspace.MakeRelative(exportFolder, wsFilename, true, true);
                exportFolder.Replace('\\', '/');
            }

            return exportFolder;
        }

        public string GetExportAbsoluteFolder(string format)
        {
            string wsFilename = Workspace.Current.FileName.Replace('/', '\\');

            string exportFolder = Workspace.Current.GetExportFolder(format);
            exportFolder = exportFolder.Replace('/', '\\');

            string exportAbsoluteFolder = FileManagers.FileManager.MakeAbsolute(wsFilename, exportFolder);
            return exportAbsoluteFolder;
        }

        public List<string> GetExportIncludedFilenames(string format)
        {
            if (_exportDatas.ContainsKey(format))
            {
                ExportData data = _exportDatas[format];
                return data.ExportIncludedFilenames;
            }

            return new List<string>();
        }

        private string MakeAbsolutePath(string relativePath)
        {
            string absolute = relativePath;

            absolute = absolute.Replace("$DESKTOP", Environment.GetFolderPath(Environment.SpecialFolder.DesktopDirectory));
            absolute = absolute.Replace("$PERSONAL", Environment.GetFolderPath(Environment.SpecialFolder.Personal));
            absolute = absolute.Replace("$DOCUMENTS", Environment.GetFolderPath(Environment.SpecialFolder.MyDocuments));
            absolute = absolute.Replace("$APPDATA", Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData));
            absolute = absolute.Replace("$LOCALAPPDATA", Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData));
            absolute = absolute.Replace("$COMMONAPPDATA", Environment.GetFolderPath(Environment.SpecialFolder.CommonApplicationData));
            absolute = absolute.Replace("$PICTURES", Environment.GetFolderPath(Environment.SpecialFolder.MyPictures));

            if (Path.IsPathRooted(absolute))
            {
                return absolute;
            }

            string path = Path.GetDirectoryName(_file_name);
            string result = Path.Combine(path, absolute);
            result = Path.GetFullPath(result);
            return result;
        }

        private string MakeRelativePath(string absolutePath)
        {
            string path = Path.GetDirectoryName(_file_name);
            string relative = MakeRelative(absolutePath, path, true, true);

            return relative;
        }

        /// <summary>
        /// Creates a new workspace.
        /// </summary>
        /// <param name="name">The name of the workspace.</param>
        /// <param name="folder">The folder the behaviors will be loaded from.</param>
        /// <param name="defaultExportFolder">The folder behaviours are exported to by default.</param>
        public Workspace(string language, string path, string name, string xmlFile, string folder, string defaultExportFolder, Dictionary<string, ExportData> exportDatas = null, int version = 3)
        {
            _file_name = Path.GetFullPath(path);
            _name = name;
            this.Language = language;

            Debug.Check(_file_name != null);
            _metaFilename = string.IsNullOrEmpty(xmlFile) ? "" : MakeAbsolutePath(xmlFile);
            _folder = MakeAbsolutePath(folder);
            _defaultExportFolder = MakeAbsolutePath(defaultExportFolder);
            _version = version;

            if (exportDatas != null)
            {
                _exportDatas = exportDatas;
            }
        }

        /// <summary>
        /// This is required for the combobox used in the workspace selection dialogue.
        /// </summary>
        /// <returns>Returns the name of the workspace.</returns>
        public override string ToString()
        {
            return _name;
        }

        /// <summary>
        /// Creates a relative path from one file
        /// or folder to another.
        /// </summary>
        /// <param name="fromDirectory">
        /// Contains the directory that defines the
        /// start of the relative path.
        /// </param>
        /// <param name="toPath">
        /// Contains the path that defines the
        /// endpoint of the relative path.
        /// </param>
        /// <returns>
        /// The relative path from the start
        /// directory to the end path.
        /// </returns>
        /// <exception cref="ArgumentNullException"></exception>
        public static string MakeRelative(string toPath, string fromDirectory_, bool bDifferent, bool from_is_path = false)
        {
            if (fromDirectory_ == null)
            { throw new ArgumentNullException("fromDirectory"); }

            string fromDirectory = fromDirectory_;

            if (!from_is_path)
            {
                fromDirectory = Path.GetDirectoryName(fromDirectory_);
            }

            if (toPath == null)
            { throw new ArgumentNullException("toPath"); }

            bool isRooted = (Path.IsPathRooted(fromDirectory) && Path.IsPathRooted(toPath));

            if (isRooted)
            {
                bool isDifferentRoot = (string.Compare(Path.GetPathRoot(fromDirectory), Path.GetPathRoot(toPath), true) != 0);

                if (isDifferentRoot)
                { return toPath; }
            }

            List<string> relativePath = new List<string>();
            string[] fromDirectories = fromDirectory.Split(Path.DirectorySeparatorChar);

            string[] toDirectories = toPath.Split(Path.DirectorySeparatorChar);

            int length = Math.Min(fromDirectories.Length, toDirectories.Length);

            int lastCommonRoot = -1;

            // find common root
            for (int x = 0; x < length; x++)
            {
                if (string.Compare(fromDirectories[x], toDirectories[x], true) != 0)
                { break; }

                lastCommonRoot = x;
            }

            if (lastCommonRoot == -1)
            { return toPath; }

            // add relative folders in from path
            int higherLevel = bDifferent ? 1 : 0;

            for (int x = lastCommonRoot + higherLevel; x < fromDirectories.Length; x++)
            {
                if (fromDirectories[x].Length > 0)
                { relativePath.Add(".."); }
            }

            // add to folders to path
            for (int x = lastCommonRoot + 1; x < toDirectories.Length; x++)
            {
                relativePath.Add(toDirectories[x]);
            }

            // create relative path
            string[] relativeParts = new string[relativePath.Count];
            relativePath.CopyTo(relativeParts, 0);

            string newPath = string.Join(Path.DirectorySeparatorChar.ToString(), relativeParts);

            if (string.IsNullOrEmpty(newPath))
            {
                newPath = ".";
            }

            return newPath;
        }

        /// <summary>
        /// Retrieves an attribute from a XML node. If the attribute does not exist an exception is thrown.
        /// </summary>
        /// <param name="node">The XML node we want to get the attribute from.</param>
        /// <param name="att">The name of the attribute we want.</param>
        /// <returns>Returns the attributes value. Is always valid.</returns>
        private static string GetAttribute(XmlNode node, string att)
        {
            XmlNode value = node.Attributes.GetNamedItem(att);

            if (value != null && value.NodeType == XmlNodeType.Attribute)
            { return value.Value; }

            return string.Empty;
        }

        public static Workspace LoadWorkspaceFile(string filename)
        {
            try
            {
                if (!File.Exists(filename))
                { return null; }

                XmlDocument xml = new XmlDocument();
                xml.Load(filename);

                XmlNode root = xml.ChildNodes[1];

                if (root.Name == "workspace")
                {
                    string language = GetAttribute(root, "language");
                    string name = GetAttribute(root, "name");
                    string folder = GetAttribute(root, "folder");
                    string defaultExportFolder = GetAttribute(root, "export");
                    string version = GetAttribute(root, "version");
                    string metaFile = GetAttribute(root, "xmlmetafolder");
                    if (string.IsNullOrEmpty(metaFile))
                    {
                        metaFile = GetAttribute(root, "metafilename");
                    }

                    if (string.IsNullOrEmpty(name) ||
                        string.IsNullOrEmpty(folder) ||
                        string.IsNullOrEmpty(defaultExportFolder))
                    {
                        throw new Exception(Resources.LoadWorkspaceError);
                    }

                    Workspace ws = new Workspace(language, filename, name, metaFile, folder, defaultExportFolder);

                    ws.Version = 0;
                    if (!string.IsNullOrEmpty(version))
                    {
                        int v = 0;
                        if (int.TryParse(version, out v))
                        {
                            ws.Version = v;
                        }
                    }

                    foreach (XmlNode subnode in root)
                    {
                        if (subnode.NodeType == XmlNodeType.Element)
                        {
                            switch (subnode.Name)
                            {
                                // Load all XMLs.
                                case "xmlmeta":
                                    string nodeName = subnode.InnerText.Trim();

                                    if (!string.IsNullOrEmpty(nodeName))
                                    { ws.MetaFilename = Path.Combine(ws.MetaFilename, nodeName); }

                                    break;

                                // Load export nodes.
                                case "export":
                                    foreach (XmlNode exportNode in subnode)
                                    {
                                        if (exportNode.NodeType == XmlNodeType.Element)
                                        {
                                            if (!ws._exportDatas.ContainsKey(exportNode.Name))
                                            {
                                                ws._exportDatas[exportNode.Name] = new ExportData();
                                            }

                                            ExportData data = ws._exportDatas[exportNode.Name];

                                            foreach (XmlNode exportInfoNode in exportNode)
                                            {
                                                switch (exportInfoNode.Name)
                                                {
                                                    case "isexported":
                                                        data.IsExported = Boolean.Parse(exportInfoNode.InnerText.Trim());
                                                        break;

                                                    case "exportunifiedfile":
                                                        bool exportUnifiedFile = Boolean.Parse(exportInfoNode.InnerText.Trim());
                                                        data.ExportFileCount = exportUnifiedFile ? 1 : -1;
                                                        break;

                                                    case "exportfilecount":
                                                        data.ExportFileCount = Int32.Parse(exportInfoNode.InnerText.Trim());
                                                        break;

                                                    case "folder":
                                                        data.ExportFolder = exportInfoNode.InnerText.Trim();
                                                        break;

                                                    case "includedfilenames":
                                                        foreach (XmlNode sn in exportInfoNode)
                                                        {
                                                            if (sn.NodeType == XmlNodeType.Element && sn.Name == "includedfilename")
                                                            {
                                                                string includeFilename = sn.InnerText.Trim();

                                                                if (!data.ExportIncludedFilenames.Contains(includeFilename))
                                                                { data.ExportIncludedFilenames.Add(includeFilename); }
                                                            }
                                                        }
                                                        break;
                                                }
                                            }
                                        }
                                    }
                                    break;
                            }
                        }
                    }

                    return ws;
                }

            }
            catch (Exception)
            {
                string msgError = string.Format(Resources.LoadWorkspaceError, filename);
                MessageBox.Show(msgError, Resources.LoadError, MessageBoxButtons.OK);
            }

            return null;
        }

        public static void SaveWorkspaceFile(Workspace ws)
        {
            // check if we have a valid result
            if (ws != null && !string.IsNullOrEmpty(ws.FileName))
            {
                try
                {
                    XmlDocument xml = new XmlDocument();

                    // Create the xml declaration.
                    XmlDeclaration declaration = xml.CreateXmlDeclaration("1.0", "utf-8", null);
                    xml.AppendChild(declaration);

                    {
                        // Create workspace node.
                        XmlElement workspace = xml.CreateElement("workspace");
                        workspace.SetAttribute("name", ws.Name);
                        if (!string.IsNullOrEmpty(ws.RelativeMetaFilename))
                        {
                            workspace.SetAttribute("metafilename", ws.RelativeMetaFilename);
                        }
                        workspace.SetAttribute("folder", ws.RelativeFolder);
                        workspace.SetAttribute("export", ws.RelativeDefaultExportFolder);
                        workspace.SetAttribute("language", ws.Language);
                        if (ws.Version > 0)
                        {
                            workspace.SetAttribute("version", ws.Version.ToString());
                        }
                        xml.AppendChild(workspace);

                        // Create export nodes.
                        XmlElement export = xml.CreateElement("export");
                        workspace.AppendChild(export);

                        foreach (string format in ws._exportDatas.Keys)
                        {
                            ExportData data = ws._exportDatas[format];

                            // Create exporter nodes.
                            XmlElement exporter = xml.CreateElement(format);
                            export.AppendChild(exporter);

                            // Create isExported node.
                            XmlElement isExported = xml.CreateElement("isexported");
                            isExported.InnerText = data.IsExported.ToString();
                            exporter.AppendChild(isExported);

                            // Create exportfilecount node.
                            if (format != "xml" && format != "bson")
                            {
                                XmlElement exportUnifiedFile = xml.CreateElement("exportfilecount");
                                exportUnifiedFile.InnerText = data.ExportFileCount.ToString();
                                exporter.AppendChild(exportUnifiedFile);
                            }

                            // Create folder node.
                            if (!string.IsNullOrEmpty(data.ExportFolder))
                            {
                                XmlElement folder = xml.CreateElement("folder");
                                folder.InnerText = data.ExportFolder;
                                exporter.AppendChild(folder);
                            }

                            // Create includedfilenames nodes.
                            if (data.ExportIncludedFilenames.Count > 0)
                            {
                                XmlElement exportincludedfilenames = xml.CreateElement("includedfilenames");
                                exporter.AppendChild(exportincludedfilenames);
                                foreach (string includedFilename in data.ExportIncludedFilenames)
                                {
                                    XmlElement exportincludedfilename = xml.CreateElement("includedfilename");
                                    exportincludedfilename.InnerText = includedFilename;
                                    exportincludedfilenames.AppendChild(exportincludedfilename);
                                }
                            }
                        }
                    }

                    //FileManagers.FileManager.MakeWritable(filename);

                    //string dir = Path.GetDirectoryName(filename);
                    //if (!string.IsNullOrEmpty(dir) && !Directory.Exists(dir))
                    //{
                    //    Directory.CreateDirectory(dir);
                    //}

                    // save workspaces
                    xml.Save(ws.FileName);

                }
                catch (Exception)
                {
                    string msgError = string.Format(Resources.SaveWorkspaceError, ws.FileName);
                    MessageBox.Show(msgError, Resources.SaveError, MessageBoxButtons.OK);
                }
            }
        }

        private string getBlackboardPath()
        {
            return Path.Combine(this.Folder, "behaviac.bb.xml");
        }

        private string getExportCustomMembersXmlPath()
        {
            return Path.Combine(this.DefaultExportFolder, "behaviac.bb.xml");
        }

        private string getExportCustomMembersBsonPath()
        {
            return Path.Combine(this.DefaultExportFolder, "behaviac.bb.bson.bytes");
        }

        private static XmlNode _agentsXMLNode = null;
        private static XmlNode _typesXMLNode = null;
        private static XmlNode _instancesXMLNode = null;

        public static XmlNode CustomizedTypesXMLNode
        {
            get { return _typesXMLNode; }
        }

        public static XmlNode CustomizedAgentsXMLNode
        {
            get { return _agentsXMLNode; }
        }

        public static XmlNode CustomizedInstancesXMLNode
        {
            get { return _instancesXMLNode; }
        }

        public static void PreLoadCustomMeta(Workspace ws)
        {
            _agentsXMLNode = null;
            _typesXMLNode = null;
            _instancesXMLNode = null;

            TypeManager.Instance.Clear();

            string bbPath = ws.getBlackboardPath();

            if (string.IsNullOrEmpty(bbPath) || !File.Exists(bbPath))
            { return; }

            XmlDocument bbfile = new XmlDocument();

            try
            {
                FileStream fs = new FileStream(bbPath, FileMode.Open, FileAccess.Read, FileShare.Read);
                bbfile.Load(fs);
                fs.Close();

                for (int i = 1; i < bbfile.ChildNodes.Count; ++i)
                {
                    XmlNode root = bbfile.ChildNodes[i];
                    if (root.Name == "meta")
                    {
                        foreach (XmlNode xmlNode in root.ChildNodes)
                        {
                            if (xmlNode.Name == "agents")
                            {
                                _agentsXMLNode = xmlNode;
                            }
                            else if (xmlNode.Name == "types")
                            {
                                _typesXMLNode = xmlNode;
                            }
                            else if (xmlNode.Name == "instances")
                            {
                                _instancesXMLNode = xmlNode;
                            }
                        }
                    }
                    else if (root.Name == "agents")
                    {
                        _agentsXMLNode = root;
                    }
                }
            }
            catch (Exception e)
            {
                string errorInfo = string.Format("{0}\n{1}", bbPath, e.Message);
                MessageBox.Show(errorInfo, Resources.LoadError, MessageBoxButtons.OK);

                bbfile.RemoveAll();
            }
        }

        public static void LoadCustomMeta(List<Nodes.Node.ErrorCheck> result)
        {
            LoadCustomTypes(_typesXMLNode);
            LoadCustomAgents(result, _agentsXMLNode);
            LoadCustomInstances(result, _instancesXMLNode);

            _agentsXMLNode = null;
            _typesXMLNode = null;
            _instancesXMLNode = null;
        }

        private static void loadCustomProperties(List<Nodes.Node.ErrorCheck> result, XmlNode propNode, AgentType agent)
        {
            Debug.Check(propNode.Name == "property" || propNode.Name == "Member");

            string propName = GetAttribute(propNode, "name");
            if (string.IsNullOrEmpty(propName))
            {
                propName = GetAttribute(propNode, "Name");
            }

            try
            {
                string isStatic = GetAttribute(propNode, "static");
                if (string.IsNullOrEmpty(isStatic))
                {
                    isStatic = GetAttribute(propNode, "Static");
                }
                bool bStatic = (!string.IsNullOrEmpty(isStatic) && isStatic == "true");

                string isPublic = GetAttribute(propNode, "public");
                if (string.IsNullOrEmpty(isPublic))
                {
                    isPublic = GetAttribute(propNode, "Public");
                }
                bool bPublic = (string.IsNullOrEmpty(isPublic) || isPublic == "true");

                string isReadonly = GetAttribute(propNode, "readonly");
                if (string.IsNullOrEmpty(isReadonly))
                {
                    isReadonly = GetAttribute(propNode, "Readonly");
                }
                bool bReadonly = (!string.IsNullOrEmpty(isReadonly) && isReadonly == "true");

                string propType = GetAttribute(propNode, "type");
                if (string.IsNullOrEmpty(propType))
                {
                    propType = GetAttribute(propNode, "TypeFullName");
                }
                Type type = Plugin.GetTypeFromName(propType);

                string classname = GetAttribute(propNode, "classname");
                if (string.IsNullOrEmpty(classname))
                {
                    classname = GetAttribute(propNode, "Class");
                }
                if (string.IsNullOrEmpty(classname))
                {
                    classname = agent.AgentTypeName;
                }

                string propDisp = GetAttribute(propNode, "disp");
                if (string.IsNullOrEmpty(propDisp))
                {
                    propDisp = GetAttribute(propNode, "DisplayName");
                }
                if (string.IsNullOrEmpty(propDisp))
                {
                    propDisp = propName;
                }

                string propDesc = GetAttribute(propNode, "desc");
                if (string.IsNullOrEmpty(propDesc))
                {
                    propDesc = GetAttribute(propNode, "Desc");
                }

                PropertyDef prop = new PropertyDef(agent, type, classname, propName, propDisp, propDesc);
                prop.IsStatic = bStatic;
                prop.IsPublic = bPublic;
                prop.IsReadonly = bReadonly;

                string defaultValue = GetAttribute(propNode, "defaultvalue");
                if (!string.IsNullOrEmpty(defaultValue))
                {
                    prop.Variable = new VariableDef(null);
                    Plugin.InvokeTypeParser(result, type, defaultValue, (object value) => prop.Variable.Value = value, null);
                }

                agent.AddProperty(prop);
            }
            catch (Exception)
            {
                string errorInfo = string.Format("error when loading Agent '{0}' Member '{1}'", agent.AgentTypeName, propName);
                MessageBox.Show(errorInfo, "Loading Custom Meta", MessageBoxButtons.OK);
            }
        }

        private static void loadCustomMethods(List<Nodes.Node.ErrorCheck> result, XmlNode methodNode, AgentType agent)
        {
            Debug.Check(methodNode.Name == "method" || methodNode.Name == "Method");

            string methodName = GetAttribute(methodNode, "name");
            if (string.IsNullOrEmpty(methodName))
            {
                methodName = GetAttribute(methodNode, "Name");
            }

            try
            {
                string returnTypename = GetAttribute(methodNode, "returntype");
                if (string.IsNullOrEmpty(returnTypename))
                {
                    returnTypename = GetAttribute(methodNode, "ReturnTypeFullName");
                }
                Type returnType = Plugin.GetTypeFromName(returnTypename);

                string isStatic = GetAttribute(methodNode, "static");
                if (string.IsNullOrEmpty(isStatic))
                {
                    isStatic = GetAttribute(methodNode, "Static");
                }
                bool bStatic = (!string.IsNullOrEmpty(isStatic) && isStatic == "true");

                string isPublic = GetAttribute(methodNode, "public");
                if (string.IsNullOrEmpty(isPublic))
                {
                    isPublic = GetAttribute(methodNode, "Public");
                }
                bool bPublic = (string.IsNullOrEmpty(isPublic) || isPublic == "true");

                string classname = GetAttribute(methodNode, "classname");
                if (string.IsNullOrEmpty(classname))
                {
                    classname = GetAttribute(methodNode, "Class");
                }
                if (string.IsNullOrEmpty(classname))
                {
                    classname = agent.AgentTypeName;
                }

                string methodDisp = GetAttribute(methodNode, "disp");
                if (string.IsNullOrEmpty(methodDisp))
                {
                    methodDisp = GetAttribute(methodNode, "DisplayName");
                }
                if (string.IsNullOrEmpty(methodDisp))
                {
                    methodDisp = methodName;
                }

                string methodDesc = GetAttribute(methodNode, "desc");
                if (string.IsNullOrEmpty(methodDesc))
                {
                    methodDesc = GetAttribute(methodNode, "Desc");
                }

                bool istask = (GetAttribute(methodNode, "istask") == "true");
                //bool isevent = (GetAttribute(methodNode, "isevent") == "true");

                MemberType memberType = MemberType.Method;
                if (istask)
                {
                    memberType = MemberType.Task;
                }

                methodName = string.Format("{0}::{1}", agent.AgentTypeName, methodName);

                MethodDef method = new MethodDef(agent, memberType, classname, methodName, methodDisp, methodDesc, "", returnType);
                method.IsStatic = bStatic;
                method.IsPublic = bPublic;

                agent.AddMethod(method);

                foreach (XmlNode paramNode in methodNode)
                {
                    string paramName = GetAttribute(paramNode, "name");
                    if (string.IsNullOrEmpty(paramName))
                    {
                        paramName = GetAttribute(paramNode, "Name");
                    }

                    string paramTypename = GetAttribute(paramNode, "type");
                    if (string.IsNullOrEmpty(paramTypename))
                    {
                        paramTypename = GetAttribute(paramNode, "TypeFullName");
                    }
                    Type paramType = Plugin.GetTypeFromName(paramTypename);

                    string isOutStr = GetAttribute(paramNode, "isout");
                    if (string.IsNullOrEmpty(isOutStr))
                    {
                        isOutStr = GetAttribute(paramNode, "IsOut");
                    }
                    bool isOut = (isOutStr == "true");

                    string isRefStr = GetAttribute(paramNode, "isref");
                    if (string.IsNullOrEmpty(isRefStr))
                    {
                        isRefStr = GetAttribute(paramNode, "IsRef");
                    }
                    bool isRef = (isRefStr == "true");

                    string nativeType = Plugin.GetNativeTypeName(paramType);

                    string paramDisp = GetAttribute(paramNode, "disp");
                    if (string.IsNullOrEmpty(paramDisp))
                    {
                        paramDisp = GetAttribute(paramNode, "DisplayName");
                    }
                    if (string.IsNullOrEmpty(paramDisp))
                    {
                        paramDisp = paramName;
                    }

                    string paramDesc = GetAttribute(paramNode, "desc");
                    if (string.IsNullOrEmpty(paramDesc))
                    {
                        paramDesc = GetAttribute(paramNode, "Desc");
                    }

                    MethodDef.Param param = new MethodDef.Param(paramName, paramType, nativeType, paramDisp, paramDesc);
                    param.IsOut = isOut;
                    param.IsRef = isRef;

                    method.Params.Add(param);
                }
            }
            catch (Exception)
            {
                string errorInfo = string.Format("error when loading Agent '{0}' Method '{1}'", agent.AgentTypeName, methodName);
                MessageBox.Show(errorInfo, "Loading Custom Meta", MessageBoxButtons.OK);
            }
        }

        private static void LoadCustomInstances(List<Nodes.Node.ErrorCheck> result, XmlNode rootNode)
        {
            try
            {
                if (rootNode == null)
                {
                    return;
                }

                foreach (XmlNode xmlNode in rootNode.ChildNodes)
                {
                    if (xmlNode.Name == "instance")
                    {
                        string name = GetAttribute(xmlNode, "name");
                        string className = GetAttribute(xmlNode, "class");
                        string disp = GetAttribute(xmlNode, "DisplayName");
                        string desc = GetAttribute(xmlNode, "Desc");

                        AgentType agent = Plugin.GetAgentType(className);
                        Debug.Check(agent != null);

                        Plugin.AddInstanceName(name, className, disp, desc, agent);
                    }
                }
            }
            catch (Exception)
            {
                string errorInfo = string.Format("error when loading custom members");

                MessageBox.Show(errorInfo, "Loading Custom Meta", MessageBoxButtons.OK);
            }
        }

        private static void LoadCustomAgents(List<Nodes.Node.ErrorCheck> result, XmlNode rootNode)
        {
            try
            {
                if (rootNode == null)
                {
                    return;
                }

                // Set the default base agent.
                if (Plugin.AgentTypes.Count == 0)
                {
                    AgentType agent = new AgentType(typeof(Agent), "Agent", false, false, "Agent", "", false);
                    Plugin.AgentTypes.Add(agent);
                }

                string agentName;

                // first pass, to load all the agents as it might be used as a property of another agent
                foreach (XmlNode xmlNode in rootNode.ChildNodes)
                {
                    if (xmlNode.Name == "agent")
                    {
                        LoadAgentType(xmlNode, out agentName);
                    }
                }

                foreach (XmlNode xmlNode in rootNode.ChildNodes)
                {
                    if (xmlNode.Name == "agent")
                    {
                        AgentType agent = LoadAgentType(xmlNode, out agentName); 
                        
                        foreach (XmlNode bbNode in xmlNode)
                        {
                            if (bbNode.Name == "properties")
                            {
                                foreach (XmlNode propNode in bbNode)
                                {
                                    loadCustomProperties(result, propNode, agent);
                                }
                            }
                            else if (bbNode.Name == "methods")
                            {
                                foreach (XmlNode methodNode in bbNode)
                                {
                                    loadCustomMethods(result, methodNode, agent);
                                }
                            }
                            else if (bbNode.Name == "Member")
                            {
                                loadCustomProperties(result, bbNode, agent);
                            }
                            else if (bbNode.Name == "Method")
                            {
                                loadCustomMethods(result, bbNode, agent);
                            }
                        }
                    }
                }
            }
            catch (Exception)
            {
                string errorInfo = string.Format("error when loading custom members");

                MessageBox.Show(errorInfo, "Loading Custom Meta", MessageBoxButtons.OK);
            }
        }

        private static AgentType LoadAgentType(XmlNode xmlNode, out string agentName)
        {
            agentName = GetAttribute(xmlNode, "type");
            if (string.IsNullOrEmpty(agentName))
            {
                agentName = GetAttribute(xmlNode, "classfullname");
            }

            AgentType agent = Plugin.GetAgentType(agentName);

            if (agent == null)
            {
                string agentBase = GetAttribute(xmlNode, "base");
                int baseIndex = -1;

                for (int i = 0; i < Plugin.AgentTypes.Count; ++i)
                {
                    if (Plugin.AgentTypes[i].AgentTypeName == agentBase)
                    {
                        baseIndex = i;
                        break;
                    }
                }

                string agentDisp = GetAttribute(xmlNode, "disp");
                if (string.IsNullOrEmpty(agentDisp))
                {
                    agentDisp = GetAttribute(xmlNode, "DisplayName");
                }

                string agentDesc = GetAttribute(xmlNode, "desc");
                if (string.IsNullOrEmpty(agentDesc))
                {
                    agentDesc = GetAttribute(xmlNode, "Desc");
                }

                if (string.IsNullOrEmpty(agentDisp))
                {
                    agentDisp = agentName;
                }

                string isExtended = GetAttribute(xmlNode, "IsExtended");

                agent = new AgentType(isExtended == "true", agentName, (baseIndex > -1) ? Plugin.AgentTypes[baseIndex] : null, agentDisp, agentDesc);
                Plugin.AgentTypes.Add(agent);
            }

            string isCustomized = GetAttribute(xmlNode, "IsCustomized");
            agent.IsCustomized = (isCustomized == "true");

            return agent;
        }

        private static void LoadCustomTypes(XmlNode rootNode)
        {
            try
            {
                if (rootNode == null)
                {
                    return;
                }

                foreach (XmlNode xmlNode in rootNode.ChildNodes)
                {
                    if (xmlNode.Name == "enumtype")
                    {
                        string enumName = GetAttribute(xmlNode, "Type");
                        EnumType enumType = TypeManager.Instance.FindEnum(enumName);
                        if (enumType != null)
                        {
                            TypeManager.Instance.Enums.Remove(enumType);
                        }

                        string[] enumNames = enumName.Split(new char[] { ':' }, StringSplitOptions.RemoveEmptyEntries);
                        enumName = enumNames[enumNames.Length - 1];

                        string isCustomized = GetAttribute(xmlNode, "IsCustomized");
                        string isExtended = GetAttribute(xmlNode, "IsExtended");
                        string ns = GetAttribute(xmlNode, "Namespace");
                        string displayName = GetAttribute(xmlNode, "DisplayName");
                        string desc = GetAttribute(xmlNode, "Desc");

                        enumType = new EnumType(isCustomized == "true", isExtended == "true", enumName, ns, displayName, desc);

                        foreach (XmlNode memberNode in xmlNode.ChildNodes)
                        {
                            if (memberNode.Name == "enum")
                            {
                                string memberName = GetAttribute(memberNode, "Value");
                                string[] memberNames = memberName.Split(new char[] { ':' }, StringSplitOptions.RemoveEmptyEntries);
                                memberName = memberNames[memberNames.Length - 1];

                                string memberDisplayName = GetAttribute(memberNode, "DisplayName");
                                string memberDesc = GetAttribute(memberNode, "Desc");
                                string memberValue = GetAttribute(memberNode, "MemberValue");

                                EnumType.EnumMemberType enumMember = new EnumType.EnumMemberType(null);
                                enumMember.Name = memberName;
                                enumMember.DisplayName = memberDisplayName;
                                enumMember.Description = memberDesc;

                                try
                                {
                                    enumMember.Value = int.Parse(memberValue);
                                }
                                catch
                                {
                                    enumMember.Value = -1;
                                }

                                enumType.Members.Add(enumMember);
                            }
                        }

                        TypeManager.Instance.Enums.Add(enumType);
                    }
                    else if (xmlNode.Name == "struct")
                    {
                        string structName = GetAttribute(xmlNode, "Type");
                        StructType structType = TypeManager.Instance.FindStruct(structName);
                        if (structType != null)
                        {
                            TypeManager.Instance.Structs.Remove(structType);
                        }

                        string[] structNames = structName.Split(new char[] { ':' }, StringSplitOptions.RemoveEmptyEntries);
                        structName = structNames[structNames.Length - 1];

                        string isCustomized = GetAttribute(xmlNode, "IsCustomized");
                        string isExtended = GetAttribute(xmlNode, "IsExtended");
                        string ns = GetAttribute(xmlNode, "Namespace");
                        string baseName = GetAttribute(xmlNode, "Base");
                        string displayName = GetAttribute(xmlNode, "DisplayName");
                        string desc = GetAttribute(xmlNode, "Desc");
                        string isStatic = GetAttribute(xmlNode, "Static");

                        structType = new StructType(isCustomized == "true", isExtended == "true", structName, ns, baseName, displayName, desc);

                        foreach (XmlNode memberNode in xmlNode.ChildNodes)
                        {
                            if (memberNode.Name == "Member")
                            {
                                string memberName = GetAttribute(memberNode, "Name");
                                string[] memberNames = memberName.Split(new char[] { ':' }, StringSplitOptions.RemoveEmptyEntries);
                                memberName = memberNames[memberNames.Length - 1];

                                string memberType = GetAttribute(memberNode, "TypeFullName");
                                if (string.IsNullOrEmpty(memberType))
                                {
                                    memberType = GetAttribute(memberNode, "Type");
                                }
                                Type type = Plugin.GetTypeFromName(memberType);

                                string memberDisplayName = GetAttribute(memberNode, "DisplayName");
                                string memberDesc = GetAttribute(memberNode, "Desc");

                                PropertyDef structProp = new PropertyDef(null, type, structName, memberName, memberDisplayName, memberDesc);
                                structProp.IsStatic = (isStatic == "true");
                                if (string.IsNullOrEmpty(structProp.NativeType))
                                {
                                    structProp.NativeType = memberType;
                                }
                                structType.Properties.Add(structProp);
                            }
                        }

                        TypeManager.Instance.Structs.Add(structType);
                    }
                }
            }
            catch (Exception)
            {
                string errorInfo = string.Format("errors when loading custom types");
                MessageBox.Show(errorInfo, "Loading Meta", MessageBoxButtons.OK);
            }
        }

        private bool _isBlackboardDirty = false;
        public bool IsBlackboardDirty
        {
            get { return _isBlackboardDirty; }
            set { _isBlackboardDirty = value; }
        }

        public static bool SaveCustomMeta(Workspace ws)
        {
            string bbPath = ws.getBlackboardPath();
            XmlDocument bbfile = new XmlDocument();

            try
            {
                FileManagers.SaveResult result = FileManagers.FileManager.MakeWritable(bbPath, Resources.SaveFileWarning);

                if (FileManagers.SaveResult.Succeeded != result)
                { return false; }

                bbfile.RemoveAll();

                XmlDeclaration declaration = bbfile.CreateXmlDeclaration("1.0", "utf-8", null);
                bbfile.AppendChild(declaration);

                //XmlComment comment = bbfile.CreateComment("EXPORTED BY TOOL, DON'T MODIFY IT!");
                //bbfile.AppendChild(comment);

                XmlElement meta = bbfile.CreateElement("meta");
                bbfile.AppendChild(meta);

                SaveCustomTypes(bbfile, meta);
                SaveCustomAgents(bbfile, meta);

                bbfile.Save(bbPath);

                ws.IsBlackboardDirty = false;

                return true;

            }
            catch (Exception ex)
            {
                bbfile.RemoveAll();

                string msgError = string.Format(Resources.SaveFileError, bbPath, ex.Message);
                MessageBox.Show(msgError, Resources.SaveError, MessageBoxButtons.OK);
            }

            return false;
        }

        private static void SaveCustomAgents(XmlDocument bbfile, XmlNode meta)
        {
            XmlElement root = bbfile.CreateElement("agents");
            meta.AppendChild(root);

            foreach (AgentType agent in Plugin.AgentTypes)
            {
                XmlElement bbEle = bbfile.CreateElement("agent");
                bbEle.SetAttribute("classfullname", agent.AgentTypeName);

                if (agent.Base != null)
                {
                    bbEle.SetAttribute("base", agent.Base.AgentTypeName);
                }

                bbEle.SetAttribute("DisplayName", agent.DisplayName);
                bbEle.SetAttribute("Desc", agent.Description);
                if (agent.IsCustomized)
                {
                    bbEle.SetAttribute("IsCustomized", "true");
                }
                if (agent.IsExtended)
                {
                    bbEle.SetAttribute("IsExtended", "true");
                }

                bool hasCustomizedProperty = false;

                foreach (PropertyDef prop in agent.GetProperties())
                {
                    if (prop.IsArrayElement || prop.IsPar)
                    {
                        continue;
                    }

                    if (agent.IsCustomized || prop.IsCustomized || prop.IsExportedButAlsoCustomized)
                    {
                        hasCustomizedProperty = true;

                        XmlElement propEle = bbfile.CreateElement("Member");

                        propEle.SetAttribute("Name", prop.BasicName);
                        propEle.SetAttribute("DisplayName", prop.DisplayName);
                        propEle.SetAttribute("Desc", prop.BasicDescription);
                        propEle.SetAttribute("Class", prop.ClassName);
                        propEle.SetAttribute("Type", prop.NativeType);
                        propEle.SetAttribute("TypeFullName", prop.Type.FullName);
                        propEle.SetAttribute("defaultvalue", prop.DefaultValue);
                        propEle.SetAttribute("Static", prop.IsStatic ? "true" : "false");
                        propEle.SetAttribute("Public", prop.IsPublic ? "true" : "false");
                        propEle.SetAttribute("Readonly", prop.IsReadonly ? "true" : "false");

                        bbEle.AppendChild(propEle);
                    }
                }

                bool hasCustomizedMethod = false;

                foreach (MethodDef method in agent.GetMethods())
                {
                    if (agent.IsCustomized || method.IsCustomized)
                    {
                        hasCustomizedMethod = true;

                        XmlElement methodEle = bbfile.CreateElement("Method");

                        methodEle.SetAttribute("Name", method.BasicName);
                        methodEle.SetAttribute("DisplayName", method.DisplayName);
                        methodEle.SetAttribute("Desc", method.BasicDescription);
                        methodEle.SetAttribute("Class", method.ClassName);
                        methodEle.SetAttribute("ReturnType", method.NativeReturnType);
                        methodEle.SetAttribute("ReturnTypeFullName", method.ReturnType.FullName);
                        methodEle.SetAttribute("Static", method.IsStatic ? "true" : "false");
                        methodEle.SetAttribute("Public", method.IsPublic ? "true" : "false");
                        methodEle.SetAttribute("istask", (method.MemberType == MemberType.Task) ? "true" : "false");
                        //methodEle.SetAttribute("isevent", (method.IsNamedEvent || method.MemberType == MemberType.Task) ? "true" : "false");

                        foreach (MethodDef.Param param in method.Params)
                        {
                            XmlElement paramEle = bbfile.CreateElement("Param");

                            paramEle.SetAttribute("Name", param.Name);
                            paramEle.SetAttribute("Type", param.NativeType);
                            paramEle.SetAttribute("TypeFullName", param.Type.FullName);

                            if (param.IsOut)
                            {
                                paramEle.SetAttribute("IsOut", "true");
                            }

                            if (param.IsRef)
                            {
                                paramEle.SetAttribute("IsRef", "true");
                            }

                            paramEle.SetAttribute("DisplayName", param.DisplayName);
                            paramEle.SetAttribute("Desc", param.Description);

                            methodEle.AppendChild(paramEle);
                        }

                        bbEle.AppendChild(methodEle);
                    }
                }

                if (hasCustomizedProperty || hasCustomizedMethod || agent.IsCustomized)
                {
                    root.AppendChild(bbEle);
                }
            }

            XmlElement instanceRoot = bbfile.CreateElement("instances");
            meta.AppendChild(instanceRoot);

            foreach (Plugin.InstanceName_t instance in Plugin.InstanceNames)
            {
                XmlElement instanceEle = bbfile.CreateElement("instance");
                instanceEle.SetAttribute("name", instance.name_);
                instanceEle.SetAttribute("class", instance.className_);
                instanceEle.SetAttribute("DisplayName", instance.displayName_);
                instanceEle.SetAttribute("Desc", instance.desc_);

                instanceRoot.AppendChild(instanceEle);
            }
        }

        private static void SaveCustomTypes(XmlDocument bbfile, XmlNode meta)
        {
            XmlElement root = bbfile.CreateElement("types");
            meta.AppendChild(root);

            foreach (EnumType enumType in TypeManager.Instance.Enums)
            {
                if (enumType.IsCustomized)
                {
                    string enumFullname = enumType.Fullname;

                    XmlElement enumEle = bbfile.CreateElement("enumtype");
                    enumEle.SetAttribute("Type", enumFullname);
                    enumEle.SetAttribute("Namespace", enumType.Namespace);
                    enumEle.SetAttribute("IsCustomized", "true");
                    if (enumType.IsExtended)
                    {
                        enumEle.SetAttribute("IsExtended", "true");
                    }
                    enumEle.SetAttribute("DisplayName", enumType.DisplayName);
                    enumEle.SetAttribute("Desc", enumType.Description);

                    foreach (EnumType.EnumMemberType member in enumType.Members)
                    {
                        XmlElement memberEle = bbfile.CreateElement("enum");

                        memberEle.SetAttribute("NativeValue", member.Name);
                        memberEle.SetAttribute("Value", member.Name);
                        memberEle.SetAttribute("DisplayName", member.DisplayName);
                        memberEle.SetAttribute("Desc", member.Description);
                        memberEle.SetAttribute("MemberValue", member.Value.ToString());

                        enumEle.AppendChild(memberEle);
                    }

                    root.AppendChild(enumEle);
                }
            }

            foreach (StructType structType in TypeManager.Instance.Structs)
            {
                if (structType.IsCustomized)
                {
                    string structFullname = structType.Fullname;

                    XmlElement structEle = bbfile.CreateElement("struct");

                    structEle.SetAttribute("Type", structFullname);
                    structEle.SetAttribute("Namespace", structType.Namespace);
                    structEle.SetAttribute("Base", structType.BaseName);
                    structEle.SetAttribute("IsCustomized", "true");
                    if (structType.IsExtended)
                    {
                        structEle.SetAttribute("IsExtended", "true");
                    }
                    structEle.SetAttribute("DisplayName", structType.DisplayName);
                    structEle.SetAttribute("Desc", structType.Description);

                    foreach (PropertyDef member in structType.Properties)
                    {
                        XmlElement memberEle = bbfile.CreateElement("Member");

                        memberEle.SetAttribute("Name", member.BasicName);
                        memberEle.SetAttribute("DisplayName", member.DisplayName);
                        memberEle.SetAttribute("Desc", member.BasicDescription);

                        Debug.Check(member.Type != null);
                        memberEle.SetAttribute("Type", member.NativeType);
                        memberEle.SetAttribute("TypeFullName", member.Type.FullName);

                        memberEle.SetAttribute("Class", structFullname);
                        memberEle.SetAttribute("Public", "true");

                        structEle.AppendChild(memberEle);
                    }

                    root.AppendChild(structEle);
                }
            }
        }

        public static void ExportCustomMembers(Workspace ws, bool exportXML, bool exportBson)
        {
            SaveCustomMeta(ws);

            if (exportXML || !exportBson)
            {
                ExportXmlCustomMembers(ws);
            }

            if (exportBson)
            {
                ExportBsonCustomMembers(ws);
            }
        }

        private static bool ExportXmlCustomMembers(Workspace ws)
        {
            string bbPath = ws.getExportCustomMembersXmlPath();

            try
            {
                FileManagers.SaveResult result = FileManagers.FileManager.MakeWritable(bbPath, Resources.SaveFileWarning);

                if (FileManagers.SaveResult.Succeeded != result)
                { return false; }

                using (StreamWriter file = new StreamWriter(bbPath))
                {
                    XmlWriterSettings xmlWs = new XmlWriterSettings();
                    xmlWs.Indent = true;
                    //xmlWs.OmitXmlDeclaration = true;

                    using (XmlWriter xmlWrtier = XmlWriter.Create(file, xmlWs))
                    {
                        xmlWrtier.WriteStartDocument();

                        xmlWrtier.WriteComment("EXPORTED BY TOOL, DON'T MODIFY IT!");
                        //bbfile.WriteComment("Source File: " + behavior.MakeRelative(behavior.FileManager.Filename));

                        xmlWrtier.WriteStartElement("agents");
                        xmlWrtier.WriteAttributeString("version", "1");
                        xmlWrtier.WriteAttributeString("signature", CRC32.CalcCRC(Plugin.Signature).ToString());

                        foreach (AgentType agent in Plugin.AgentTypes)
                        {
                            bool hasCustomizedProperty = false;
                            foreach (PropertyDef prop in agent.GetProperties())
                            {
                                //skip array element
                                if (prop.IsArrayElement || prop.IsPar || prop.IsMember)
                                {
                                    continue;
                                }

                                hasCustomizedProperty = true;
                                break;
                            }

                            IList<MethodDef> methods = agent.GetMethods();
                            bool hasCustomizedMethod = false;
                            //foreach (MethodDef method in methods)
                            //{
                            //    if (method.IsNamedEvent)
                            //    {
                            //        hasCustomizedMethod = true;
                            //        break;
                            //    }
                            //}

                            if (agent.IsCustomized || hasCustomizedProperty || hasCustomizedMethod)
                            {
                                xmlWrtier.WriteStartElement("agent");

                                xmlWrtier.WriteAttributeString("type", agent.AgentTypeName);

                                if (agent.Base != null)
                                {
                                    xmlWrtier.WriteAttributeString("base", agent.Base.AgentTypeName);
                                }

                                if (hasCustomizedProperty)
                                {
                                    xmlWrtier.WriteStartElement("properties");

                                    foreach (PropertyDef prop in agent.GetProperties())
                                    {
                                        //skip array element
                                        if (prop.IsArrayElement || prop.IsPar || prop.IsMember)
                                        {
                                            continue;
                                        }

                                        xmlWrtier.WriteStartElement("property");

                                        xmlWrtier.WriteAttributeString("name", prop.BasicName);
                                        xmlWrtier.WriteAttributeString("type", Plugin.GetNativeTypeName(prop.NativeType));
                                        xmlWrtier.WriteAttributeString("member", prop.IsMember ? "true" : "false");
                                        xmlWrtier.WriteAttributeString("static", prop.IsStatic ? "true" : "false");
                                        //xmlWrtier.WriteAttributeString("public", prop.IsPublic ? "true" : "false");

                                        if (prop.IsMember)
                                        {
                                            xmlWrtier.WriteAttributeString("agent", prop.ClassName);
                                        }
                                        else
                                        {
                                            xmlWrtier.WriteAttributeString("defaultvalue", prop.DefaultValue);
                                        }

                                        xmlWrtier.WriteEndElement();
                                    }

                                    xmlWrtier.WriteEndElement();
                                }

                                /*
                                if (hasCustomizedMethod)
                                {
                                    xmlWrtier.WriteStartElement("methods");

                                    foreach (MethodDef method in methods)
                                    {
                                        //skip root, but custom method is included
                                        if (method.IsNamedEvent)
                                        {
                                            xmlWrtier.WriteStartElement("method");

                                            xmlWrtier.WriteAttributeString("name", method.BasicName);
                                            //xmlWrtier.WriteAttributeString("returntype", Plugin.GetNativeTypeName(method.ReturnType));
                                            //xmlWrtier.WriteAttributeString("static", method.IsStatic ? "true" : "false");
                                            //xmlWrtier.WriteAttributeString("public", method.IsPublic ? "true" : "false");
                                            //xmlWrtier.WriteAttributeString("istask", (method.MemberType == MemberType.Task) ? "true" : "false");
                                            //xmlWrtier.WriteAttributeString("isevent", (method.IsNamedEvent || method.MemberType == MemberType.Task) ? "true" : "false");
                                            xmlWrtier.WriteAttributeString("agent", method.ClassName);

                                            foreach (MethodDef.Param param in method.Params)
                                            {
                                                xmlWrtier.WriteStartElement("parameter");

                                                xmlWrtier.WriteAttributeString("name", param.Name);
                                                xmlWrtier.WriteAttributeString("type", param.NativeType);

                                                xmlWrtier.WriteEndElement();
                                            }

                                            xmlWrtier.WriteEndElement();
                                        }
                                    }

                                    xmlWrtier.WriteEndElement();
                                }
                                */

                                //end of agent
                                xmlWrtier.WriteEndElement();
                            }
                        }

                        //end of agents
                        xmlWrtier.WriteEndElement();

                        xmlWrtier.WriteEndDocument();
                    }

                    file.Close();
                }

                return true;

            }
            catch (Exception ex)
            {
                string msgError = string.Format(Resources.SaveFileError, bbPath, ex.Message);
                MessageBox.Show(msgError, Resources.SaveError, MessageBoxButtons.OK);
            }

            return false;
        }

        private static bool ExportBsonCustomMembers(Workspace ws)
        {
            string bbPath = ws.getExportCustomMembersBsonPath();

            try
            {
                FileManagers.SaveResult result = FileManagers.FileManager.MakeWritable(bbPath, Resources.SaveFileWarning);

                if (FileManagers.SaveResult.Succeeded != result)
                { return false; }

                using (var ms = new MemoryStream())
                using (var writer = new BinaryWriter(ms))
                {
                    BsonSerializer serializer = BsonSerializer.CreateSerialize(writer);
                    serializer.WriteStartDocument();

                    serializer.WriteComment("EXPORTED BY TOOL, DON'T MODIFY IT!");

                    serializer.WriteStartElement("agents");
                    serializer.WriteString("1"); // version
                    serializer.WriteString(CRC32.CalcCRC(Plugin.Signature).ToString()); // signature

                    foreach (AgentType agent in Plugin.AgentTypes)
                    {
                        bool hasCustomizedProperty = false;
                        foreach (PropertyDef prop in agent.GetProperties())
                        {
                            //skip array element
                            if (prop.IsArrayElement || prop.IsPar || prop.IsMember)
                            {
                                continue;
                            }

                            hasCustomizedProperty = true;
                            break;
                        }

                        //IList<MethodDef> methods = agent.GetMethods(MethodType.Task | MethodType.Event);
                        IList<MethodDef> methods = agent.GetMethods();
                        bool hasCustomizedMethod = false;
                        //foreach (MethodDef method in methods)
                        //{
                        //    if (method.IsNamedEvent)
                        //    {
                        //        hasCustomizedMethod = true;
                        //        break;
                        //    }
                        //}

                        if (agent.IsCustomized || hasCustomizedProperty || hasCustomizedMethod)
                        {
                            serializer.WriteStartElement("agent");

                            serializer.WriteString(agent.AgentTypeName);

                            //base
                            if (agent.Base != null)
                            {
                                serializer.WriteString(agent.Base.AgentTypeName);
                            }
                            else
                            {
                                serializer.WriteString("none");
                            }

                            if (hasCustomizedProperty)
                            {
                                serializer.WriteStartElement("properties");

                                foreach (PropertyDef prop in agent.GetProperties())
                                {
                                    //skip array element
                                    if (prop.IsArrayElement || prop.IsPar || prop.IsMember)
                                    {
                                        continue;
                                    }

                                    serializer.WriteStartElement("property");

                                    serializer.WriteString(prop.BasicName);
                                    serializer.WriteString(Plugin.GetNativeTypeName(prop.NativeType));
                                    serializer.WriteString(prop.IsMember ? "true" : "false");
                                    serializer.WriteString(prop.IsStatic ? "true" : "false");
                                    //serializer.WriteString(prop.IsPublic ? "true" : "false");

                                    if (prop.IsMember)
                                    {
                                        serializer.WriteString(prop.ClassName);
                                    }
                                    else
                                    {
                                        serializer.WriteString(prop.DefaultValue);
                                    }

                                    serializer.WriteEndElement();
                                }

                                serializer.WriteEndElement();
                            }

                            /*
                            if (hasCustomizedMethod)
                            {
                                serializer.WriteStartElement("methods");

                                foreach (MethodDef method in methods)
                                {
                                    //skip root and those other custom method
                                    if (method.IsNamedEvent)
                                    {
                                        serializer.WriteStartElement("method");

                                        serializer.WriteString(method.BasicName);
                                        //serializer.WriteString(Plugin.GetNativeTypeName(method.ReturnType));
                                        //serializer.WriteString(method.IsStatic ? "true" : "false");
                                        //serializer.WriteString(method.IsPublic ? "true" : "false");
                                        //serializer.WriteString((method.MemberType == MemberType.Task) ? "true" : "false");
                                        serializer.WriteString(method.ClassName);

                                        foreach (MethodDef.Param param in method.Params)
                                        {
                                            serializer.WriteStartElement("parameter");

                                            serializer.WriteString(param.Name);
                                            serializer.WriteString(param.NativeType);

                                            serializer.WriteEndElement();
                                        }

                                        serializer.WriteEndElement();
                                    }
                                }

                                //end of methods
                                serializer.WriteEndElement();
                            }
                            */

                            //end of agent
                            serializer.WriteEndElement();
                        }
                        else
                        {
                            //serializer.WriteTypeNone();
                        }
                    }

                    //end of 'agents'
                    serializer.WriteEndElement();

                    serializer.WriteEndDocument();

                    using (FileStream fs = File.Create(bbPath))
                    {
                        using (BinaryWriter w = new BinaryWriter(fs))
                        {
                            byte[] d = ms.ToArray();

                            w.Write(d);
                            fs.Close();
                        }
                    }
                }

                return true;
            }
            catch (Exception ex)
            {
                string msgError = string.Format(Resources.SaveFileError, bbPath, ex.Message);
                MessageBox.Show(msgError, Resources.SaveError, MessageBoxButtons.OK);
            }

            return false;
        }
    }
}
