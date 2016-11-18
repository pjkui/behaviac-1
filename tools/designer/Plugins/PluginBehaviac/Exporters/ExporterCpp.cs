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
using System.Text;
using System.IO;
using Behaviac.Design;
using Behaviac.Design.Nodes;
using Behaviac.Design.Attachments;
using Behaviac.Design.Attributes;
using PluginBehaviac.Properties;
using PluginBehaviac.NodeExporters;
using PluginBehaviac.DataExporters;

namespace PluginBehaviac.Exporters
{
    public class ExporterCpp : Behaviac.Design.Exporters.Exporter
    {
        class BehaviorCreator
        {
            public BehaviorCreator(string filename, string classname)
            {
                Filename = filename;
                Classname = classname;
            }

            public string Filename;
            public string Classname;
        }

        List<BehaviorCreator> _behaviorCreators = new List<BehaviorCreator>();

        public ExporterCpp(BehaviorNode node, string outputFolder, string filename, List<string> includedFilenames = null)
            : base(node, outputFolder, filename, includedFilenames)
        {
            //automatically create an extra level of path
            _outputFolder = Path.Combine(Path.GetFullPath(_outputFolder), "behaviac_generated");
            _filename = "behaviors/behaviac_generated_behaviors.h";
        }

        public override Behaviac.Design.FileManagers.SaveResult Export(List<BehaviorNode> behaviors, bool exportBehaviors, int exportFileCount)
        {
            string behaviorFilename = "behaviors/behaviac_generated_behaviors.h";
            string typesFolder = string.Empty;
            Behaviac.Design.FileManagers.SaveResult result = VerifyFilename(exportBehaviors, ref behaviorFilename, ref typesFolder);
            if (Behaviac.Design.FileManagers.SaveResult.Succeeded == result)
            {
                // meta
                ExportTypesHeader(typesFolder);

                string internalFolder = Path.Combine(typesFolder, "internal");
                if (!Directory.Exists(internalFolder))
                {
                    Directory.CreateDirectory(internalFolder);
                }

                ExportAgentHeader(internalFolder);
                ExportAgentMeta(internalFolder);
                ExportAgentMemberVisitor(internalFolder);

                ExportAgentsDefinition(internalFolder);
                ExportAgentsImplemention(internalFolder);

                ExportCustomizedTypesDefinition(internalFolder);
                ExportCustomizedTypesImplemention(internalFolder);

                // behaviors
                if (exportBehaviors)
                {
                    string behaviorFolder = Path.GetDirectoryName(behaviorFilename);
                    clearFolder(behaviorFolder);
                    //clearFolder(agentFolder);

                    ExportBehaviors(behaviors, behaviorFilename, exportFileCount);
                }
            }

            return result;
        }

        private void clearFolder(string folder)
        {
            DirectoryInfo dirInfo = new DirectoryInfo(folder);
            if (dirInfo.Exists)
            {
                foreach (FileInfo file in dirInfo.GetFiles("*.*", SearchOption.AllDirectories))
                {
                    try
                    {
                        File.SetAttributes(file.FullName, FileAttributes.Normal);
                        if (file.Extension == ".inl" || file.Extension == ".h" || file.Extension == ".cpp")
                            file.Delete();
                    }
                    catch
                    {
                    }
                }
            }
        }

        private void ExportBehaviors(List<BehaviorNode> behaviors, string filename, int exportFileCount)
        {
            using (StreamWriter file = new StreamWriter(filename)) // behaviac_generated_behaviors.h
            {
                _behaviorCreators.Clear();

                string cppFileName = Path.ChangeExtension(filename, ".cpp");

                using (StreamWriter baseCppFile = new StreamWriter(cppFileName)) // behaviac_generated_behaviors.cpp
                {
                    baseCppFile.WriteLine("// ---------------------------------------------------------------------");
                    baseCppFile.WriteLine("// This file is auto-generated by behaviac designer, so please don't modify it by yourself!");
                    baseCppFile.WriteLine("// ---------------------------------------------------------------------");
                    baseCppFile.WriteLine();

                    baseCppFile.WriteLine("#include \"behaviac/behaviac.h\"");
                    baseCppFile.WriteLine();
                    baseCppFile.WriteLine("#include \"behaviac_generated_behaviors.h\"");

                    if (exportFileCount <= 1)
                    {
                        file.WriteLine("// ---------------------------------------------------------------------");
                        file.WriteLine("// This file is auto-generated by behaviac designer, so please don't modify it by yourself!");
                        file.WriteLine("// ---------------------------------------------------------------------");
                        file.WriteLine();

                        string headerFileMacro = "_BEHAVIAC_GENERATED_BEHAVIORS_H_";

                        file.WriteLine("#ifndef {0}", headerFileMacro);
                        file.WriteLine("#define {0}", headerFileMacro);
                        file.WriteLine();

                        file.WriteLine("#include \"../types/behaviac_types.h\"");
                        file.WriteLine();

                        file.WriteLine("namespace behaviac");
                        file.WriteLine("{");

                        foreach (BehaviorNode behavior in behaviors)
                        {
                            behavior.PreExport();

                            _behaviorCreators.Add(ExportBody(file, behavior, false));

                            behavior.PostExport();
                        }

                        file.WriteLine("}");
                        file.WriteLine("#endif // {0}", headerFileMacro);
                    }
                    else
                    {
                        string ext = Path.GetExtension(filename);
                        int unitNum = 0;
                        int behaviorsCount = behaviors.Count;
                        int behaviorUnitSize = behaviorsCount / exportFileCount;
                        if (behaviorUnitSize < 1)
                            behaviorUnitSize = 1;

                        for (int i = 0; i < behaviorsCount; i += behaviorUnitSize)
                        {
                            Debug.Check(unitNum < 10000);

                            string unitHeaderFileName = filename.Replace(ext, "_" + unitNum + ".h");
                            string unitCppFileName = filename.Replace(ext, "_" + unitNum + ".cpp");

                            StreamWriter headerSW = new StreamWriter(unitHeaderFileName);
                            StreamWriter cppSW = new StreamWriter(unitCppFileName);

                            // header
                            headerSW.WriteLine("// ---------------------------------------------------------------------");
                            headerSW.WriteLine("// This file is auto-generated by behaviac designer, so please don't modify it by yourself!");
                            headerSW.WriteLine("// ---------------------------------------------------------------------");
                            headerSW.WriteLine();
                            headerSW.WriteLine("#include \"../types/behaviac_types.h\"");
                            headerSW.WriteLine();

                            headerSW.WriteLine("namespace behaviac");
                            headerSW.WriteLine("{");

                            // cpp
                            cppSW.WriteLine("// ---------------------------------------------------------------------");
                            cppSW.WriteLine("// This file is auto-generated by behaviac designer, so please don't modify it by yourself!");
                            cppSW.WriteLine("// ---------------------------------------------------------------------");
                            cppSW.WriteLine();

                            string unitHeaderFile = Path.GetFileName(unitHeaderFileName);
                            baseCppFile.WriteLine("#include \"{0}\"", unitHeaderFile);

                            cppSW.WriteLine("#include \"{0}\"", unitHeaderFile);
                            cppSW.WriteLine();
                            cppSW.WriteLine("namespace behaviac");
                            cppSW.WriteLine("{");

                            for (int k = 0; k < behaviorUnitSize && i + k < behaviorsCount; ++k)
                            {
                                BehaviorNode behavior = behaviors[i + k];

                                behavior.PreExport();

                                // cpp
                                BehaviorCreator creator = ExportBody(cppSW, behavior, true);
                                _behaviorCreators.Add(creator);

                                // header
                                headerSW.WriteLine("\tclass {0}", creator.Classname);
                                headerSW.WriteLine("\t{");
                                headerSW.WriteLine("\tpublic:");
                                headerSW.WriteLine("\t\tstatic bool Create(BehaviorTree* pBT);");
                                headerSW.WriteLine("\t};");
                                headerSW.WriteLine();

                                behavior.PostExport();
                            }

                            // header
                            headerSW.WriteLine("}");
                            headerSW.Close();

                            // cpp
                            cppSW.WriteLine("}");
                            cppSW.Close();

                            unitNum++;
                        }
                    }

                    baseCppFile.WriteLine();
                    baseCppFile.WriteLine("namespace behaviac");
                    baseCppFile.WriteLine("{");

                    ExportTail(baseCppFile);

                    baseCppFile.WriteLine("}");
                    baseCppFile.Close();
                }

                file.Close();
            }
        }

        private Behaviac.Design.FileManagers.SaveResult VerifyFilename(bool exportBehaviors, ref string behaviorFilename, ref string typesFolder)
        {
            behaviorFilename = Path.Combine(_outputFolder, behaviorFilename);
            typesFolder = Path.Combine(_outputFolder, "types");

            // get the abolute folder of the file we want to export
            string folder = Path.GetDirectoryName(behaviorFilename);
            if (!Directory.Exists(folder))
                Directory.CreateDirectory(folder);

            if (!Directory.Exists(typesFolder))
                Directory.CreateDirectory(typesFolder);

            if (exportBehaviors)
            {
                // verify it can be writable
                return Behaviac.Design.FileManagers.FileManager.MakeWritable(behaviorFilename, Resources.ExportFileWarning);
            }

            return Behaviac.Design.FileManagers.SaveResult.Succeeded;
        }

        private List<string> GetNamespaces(string ns)
        {
            List<string> namespaces = new List<string>();
            int startIndex = 0;

            for (int i = 0; i < ns.Length; ++i)
            {
                if (ns[i] == ':')
                {
                    Debug.Check(ns[i + 1] == ':');

                    namespaces.Add(ns.Substring(startIndex, i - startIndex));
                    startIndex = i + 2;
                    ++i;
                }
            }

            namespaces.Add(ns.Substring(startIndex, ns.Length - startIndex));

            return namespaces;
        }

        private string WriteNamespacesHead(StreamWriter file, List<string> namespaces)
        {
            string indent = string.Empty;
            for (int i = 0; i < namespaces.Count; ++i)
            {
                file.WriteLine("{0}namespace {1}", indent, namespaces[i]);
                file.WriteLine("{0}{{", indent);
                indent += '\t';
            }

            return indent;
        }

        private void WriteNamespacesTail(StreamWriter file, List<string> namespaces)
        {
            for (int i = 0; i < namespaces.Count; ++i)
            {
                string indent = string.Empty;
                for (int k = i + 1; k < namespaces.Count; ++k)
                {
                    indent += '\t';
                }

                file.WriteLine("{0}}}\r\n", indent);
            }
        }

        private void ExportBehaviacHeader(StreamWriter file)
        {
            file.WriteLine("#include \"behaviac/behaviac.h\"");
            file.WriteLine("#include \"behaviac/behaviortree/behaviortree.h\"");
            file.WriteLine("#include \"behaviac/agent/agent.h\"");
            file.WriteLine("#include \"behaviac/common/meta.h\"");
            file.WriteLine();

            file.WriteLine("#include \"behaviac/behaviortree/nodes/actions/action.h\"");
            file.WriteLine("#include \"behaviac/behaviortree/nodes/actions/assignment.h\"");
            file.WriteLine("#include \"behaviac/behaviortree/nodes/actions/compute.h\"");
            file.WriteLine("#include \"behaviac/behaviortree/nodes/actions/noop.h\"");
            file.WriteLine("#include \"behaviac/behaviortree/nodes/actions/wait.h\"");
            file.WriteLine("#include \"behaviac/behaviortree/nodes/actions/waitforsignal.h\"");
            file.WriteLine("#include \"behaviac/behaviortree/nodes/actions/waitframes.h\"");
            file.WriteLine("#include \"behaviac/behaviortree/nodes/composites/compositestochastic.h\"");
            file.WriteLine("#include \"behaviac/behaviortree/nodes/composites/ifelse.h\"");
            file.WriteLine("#include \"behaviac/behaviortree/nodes/composites/parallel.h\"");
            file.WriteLine("#include \"behaviac/behaviortree/nodes/composites/referencebehavior.h\"");
            file.WriteLine("#include \"behaviac/behaviortree/nodes/composites/selector.h\"");
            file.WriteLine("#include \"behaviac/behaviortree/nodes/composites/selectorloop.h\"");
            file.WriteLine("#include \"behaviac/behaviortree/nodes/composites/selectorprobability.h\"");
            file.WriteLine("#include \"behaviac/behaviortree/nodes/composites/selectorstochastic.h\"");
            file.WriteLine("#include \"behaviac/behaviortree/nodes/composites/sequence.h\"");
            file.WriteLine("#include \"behaviac/behaviortree/nodes/composites/sequencestochastic.h\"");
            file.WriteLine("#include \"behaviac/behaviortree/nodes/composites/withprecondition.h\"");
            file.WriteLine("#include \"behaviac/behaviortree/nodes/conditions/and.h\"");
            file.WriteLine("#include \"behaviac/behaviortree/nodes/conditions/conditionbase.h\"");
            file.WriteLine("#include \"behaviac/behaviortree/nodes/conditions/condition.h\"");
            file.WriteLine("#include \"behaviac/behaviortree/nodes/conditions/false.h\"");
            file.WriteLine("#include \"behaviac/behaviortree/nodes/conditions/or.h\"");
            file.WriteLine("#include \"behaviac/behaviortree/nodes/conditions/true.h\"");
            file.WriteLine("#include \"behaviac/behaviortree/nodes/decorators/decoratoralwaysfailure.h\"");
            file.WriteLine("#include \"behaviac/behaviortree/nodes/decorators/decoratoralwaysrunning.h\"");
            file.WriteLine("#include \"behaviac/behaviortree/nodes/decorators/decoratoralwayssuccess.h\"");
            file.WriteLine("#include \"behaviac/behaviortree/nodes/decorators/decoratorcount.h\"");
            file.WriteLine("#include \"behaviac/behaviortree/nodes/decorators/decoratorcountlimit.h\"");
            file.WriteLine("#include \"behaviac/behaviortree/nodes/decorators/decoratorfailureuntil.h\"");
            file.WriteLine("#include \"behaviac/behaviortree/nodes/decorators/decoratorframes.h\"");
            file.WriteLine("#include \"behaviac/behaviortree/nodes/decorators/decoratoriterator.h\"");
            file.WriteLine("#include \"behaviac/behaviortree/nodes/decorators/decoratorlog.h\"");
            file.WriteLine("#include \"behaviac/behaviortree/nodes/decorators/decoratorloop.h\"");
            file.WriteLine("#include \"behaviac/behaviortree/nodes/decorators/decoratorloopuntil.h\"");
            file.WriteLine("#include \"behaviac/behaviortree/nodes/decorators/decoratornot.h\"");
            file.WriteLine("#include \"behaviac/behaviortree/nodes/decorators/decoratorrepeat.h\"");
            file.WriteLine("#include \"behaviac/behaviortree/nodes/decorators/decoratorsuccessuntil.h\"");
            file.WriteLine("#include \"behaviac/behaviortree/nodes/decorators/decoratortime.h\"");
            file.WriteLine("#include \"behaviac/behaviortree/nodes/decorators/decoratorweight.h\"");
            file.WriteLine("#include \"behaviac/behaviortree/attachments/event.h\"");
            file.WriteLine("#include \"behaviac/behaviortree/attachments/attachaction.h\"");
            file.WriteLine("#include \"behaviac/behaviortree/attachments/precondition.h\"");
            file.WriteLine("#include \"behaviac/behaviortree/attachments/effector.h\"");
            file.WriteLine("#include \"behaviac/htn/task.h\"");
            file.WriteLine("#include \"behaviac/fsm/fsm.h\"");
            file.WriteLine("#include \"behaviac/fsm/state.h\"");
            file.WriteLine("#include \"behaviac/fsm/startcondition.h\"");
            file.WriteLine("#include \"behaviac/fsm/transitioncondition.h\"");
            file.WriteLine("#include \"behaviac/fsm/waitstate.h\"");
            file.WriteLine("#include \"behaviac/fsm/waitframesstate.h\"");
            file.WriteLine("#include \"behaviac/fsm/alwaystransition.h\"");
            file.WriteLine("#include \"behaviac/fsm/waittransition.h\"");
        }

        private void ExportTypesHeader(string agentFolder)
        {
            Encoding utf8WithBom = new UTF8Encoding(true);
            const string kTypesFilename = "behaviac_types.h";
            string filename = Path.Combine(agentFolder, kTypesFilename);

            using (StreamWriter file = new StreamWriter(filename, false, utf8WithBom))
            {
                file.WriteLine("// ---------------------------------------------------------------------");
                file.WriteLine("// This file is auto-generated by behaviac designer, so please don't modify it by yourself!");
                file.WriteLine("// ---------------------------------------------------------------------");
                file.WriteLine();

                string headerFileMacro = "_BEHAVIAC_TYPES_H_";

                file.WriteLine("#ifndef {0}", headerFileMacro);
                file.WriteLine("#define {0}", headerFileMacro);
                file.WriteLine();

                ExportBehaviacHeader(file);
                file.WriteLine();

                file.WriteLine("#include \"internal/behaviac_agent_headers.h\"");
                file.WriteLine("#include \"internal/behaviac_agent_member_visitor.h\"");
                if (TypeManager.Instance.HasCustomizedTypes())
                {
                    file.WriteLine("#include \"internal/behaviac_customized_types.h\"");
                }

                file.WriteLine();
                file.WriteLine("#endif // {0}", headerFileMacro);

                file.Close();
            }
        }

        private void ExportAgentHeader(string agentFolder)
        {
            Encoding utf8WithBom = new UTF8Encoding(true);
            const string kAgentHeadersFilename = "behaviac_agent_headers.h";
            string filename = Path.Combine(agentFolder, kAgentHeadersFilename);

            using (StreamWriter file = new StreamWriter(filename, false, utf8WithBom))
            {
                file.WriteLine("// ---------------------------------------------------------------------");
                file.WriteLine("// This file is auto-generated by behaviac designer, so please don't modify it by yourself!");
                file.WriteLine("// ---------------------------------------------------------------------");
                file.WriteLine();

                string headerFileMacro = "_BEHAVIAC_HEADERS_H_";

                file.WriteLine("#ifndef {0}", headerFileMacro);
                file.WriteLine("#define {0}", headerFileMacro);
                file.WriteLine();

                file.WriteLine("#include \"behaviac/behaviac.h\"");
                file.WriteLine();

                file.WriteLine("// You should set the agent header files of your game when exporting cpp files on the behaviac editor:");

                // write included files for the game agents
                if (this.IncludedFilenames != null)
                {
                    foreach (string headerFilename in this.IncludedFilenames)
                    {
                        file.WriteLine("#include \"../../../{0}\"", headerFilename);
                    }
                }

                file.WriteLine();
                file.WriteLine("// The following agent header files is generated automatically:");

                foreach (AgentType agent in Plugin.AgentTypes)
                {
                    if (!agent.IsCustomized)
                        continue;

                    file.WriteLine("#include \"{0}.h\"", agent.BasicClassName);
                }

                file.WriteLine();
                file.WriteLine("#endif // {0}", headerFileMacro);

                file.Close();
            }
        }

        private void ExportAgentMemberVisitor(string folder)
        {
            Encoding utf8WithBom = new UTF8Encoding(true);
            string filename = Path.Combine(folder, "behaviac_agent_member_visitor.h");

            using (StreamWriter file = new StreamWriter(filename, false, utf8WithBom))
            {
                file.WriteLine("// ---------------------------------------------------------------------");
                file.WriteLine("// This file is auto-generated by behaviac designer, so please don't modify it by yourself!");
                file.WriteLine("// ---------------------------------------------------------------------");
                file.WriteLine();

                string headerFileMacro = "_BEHAVIAC_MEMBER_VISITOR_H_";

                file.WriteLine("#ifndef {0}", headerFileMacro);
                file.WriteLine("#define {0}", headerFileMacro);
                file.WriteLine();

                file.WriteLine("#include \"behaviac_agent_headers.h\"");
                file.WriteLine();

                GenerateMemberHandler(file);

                file.WriteLine("#endif // {0}", headerFileMacro);
            }
        }

        private void GenerateMemberHandler(StreamWriter file)
        {
            // write property and method handlers
            file.WriteLine("// Agent property and method handlers\r\n");
            string allParamTypes = string.Empty;

            foreach (AgentType agenType in Plugin.AgentTypes)
            {
                List<string> namespaces = new List<string>();
                string ns = agenType.Namespace;

                if (!string.IsNullOrEmpty(ns))
                {
                    foreach (PropertyDef property in agenType.GetProperties())
                    {
                        if (property.IsMember && !property.IsAddedAutomatically)
                        {
                            namespaces = GetNamespaces(ns);
                            break;
                        }
                    }
                }

                if (namespaces.Count == 0 && !string.IsNullOrEmpty(ns))
                {
                    foreach (MethodDef method in agenType.GetMethods())
                    {
                        if (method.ClassName == agenType.AgentTypeName)
                        {
                            namespaces = GetNamespaces(ns);
                            break;
                        }
                    }
                }

                string indent = WriteNamespacesHead(file, namespaces);

                foreach (PropertyDef property in agenType.GetProperties())
                {
                    if (!property.IsPublic && property.IsMember && !property.IsAddedAutomatically)
                    {
                        string propName = property.AgentType.AgentTypeName.Replace("::", "_") + "_" + property.BasicName.Replace("::", "_").Replace("[]", "");
                        string nativeType = DataCppExporter.GetBasicGeneratedNativeType(property.NativeType);
                        if (nativeType.IndexOf("string") != -1)
                        {
                            //Console.WriteLine("hello");
                        }
                        file.WriteLine("{0}struct PROPERTY_TYPE_{1} {{ }};", indent, propName);
                        file.WriteLine("{0}template<> inline {1}& {2}::_Get_Property_<PROPERTY_TYPE_{3}>()", indent, nativeType, agenType.BasicClassName, propName);
                        file.WriteLine("{0}{{", indent);
                        if (property.IsProperty)
                        {
                            file.WriteLine("{0}\treturn *({1}*)&this->GetVariable<{1}>(\"{2}\");", indent, nativeType, property.BasicName);
                        }
                        else // field
                        {
                            if (property.IsStatic)
                            {
                                file.WriteLine("{0}\tunsigned char* pc = (unsigned char*)(&{1});", indent, property.Name);
                            }
                            else
                            {
                                file.WriteLine("{0}\tunsigned char* pc = (unsigned char*)this;", indent);
                                file.WriteLine("{0}\tpc += (int)BEHAVIAC_OFFSETOF({1}, {2});", indent, property.ClassName, property.Name);
                            }
                            file.WriteLine("{0}\treturn *(reinterpret_cast<{1}*>(pc));", indent, nativeType);
                        }

                        file.WriteLine("{0}}}\r\n", indent);
                    }
                }

                foreach (MethodDef method in agenType.GetMethods())
                {
                    if (!method.IsPublic && !method.IsCustomized && method.ClassName == agenType.AgentTypeName)
                    {
                        string paramStrDef = string.Empty;
                        string paramStr = string.Empty;
                        for (int i = 0; i < method.Params.Count; ++i)
                        {
                            if (i > 0)
                            {
                                paramStrDef += ", ";
                                paramStr += ", ";
                            }

                            string basicNativeType = DataCppExporter.GetGeneratedNativeType(method.Params[i].NativeType);
                            paramStrDef += string.Format("{0} p{1}", basicNativeType, i);
                            paramStr += string.Format("p{0}", i);
                        }
                        string methodName = agenType.AgentTypeName.Replace("::", "_") + "_" + method.BasicName.Replace("::", "_");
                        string nativeReturnType = DataCppExporter.GetGeneratedNativeType(method.NativeReturnType);
                        if (method.NativeReturnType.StartsWith("const "))
                            nativeReturnType = "const " + nativeReturnType;

                        file.WriteLine("{0}struct METHOD_TYPE_{1} {{ }};", indent, methodName);
                        file.WriteLine("{0}template<> inline {1} {2}::_Execute_Method_<METHOD_TYPE_{3}>({4})", indent, nativeReturnType, agenType.BasicClassName, methodName, paramStrDef);
                        file.WriteLine("{0}{{", indent);

                        string ret = (method.NativeReturnType == "void") ? string.Empty : "return ";
                        file.WriteLine("{0}\t{1}this->{2}({3});", indent, ret, method.Name, paramStr);

                        file.WriteLine("{0}}}\r\n", indent);
                    }
                }

                WriteNamespacesTail(file, namespaces);
            }
        }

        private BehaviorCreator ExportBody(StreamWriter file, BehaviorNode behavior, bool onlyImplement)
        {
            string filename = Path.ChangeExtension(behavior.RelativePath, "").Replace(".", "");
            filename = filename.Replace('\\', '/');

            // write comments
            file.WriteLine("\t// Source file: {0}\r\n", filename);

            string btClassName = string.Format("bt_{0}", filename.Replace('/', '_'));
            string agentType = behavior.AgentType.AgentTypeName;

            // create the class definition of its attachments
            ExportAttachmentClass(file, btClassName, (Node)behavior);

            // create the class definition of its children
            foreach (Node child in ((Node)behavior).GetChildNodes())
                ExportNodeClass(file, btClassName, agentType, behavior, child);

            // export the create function
            if (onlyImplement)
            {
                file.WriteLine("\t\tbool {0}::Create(BehaviorTree* pBT)", btClassName);
            }
            else
            {
                file.WriteLine("\tclass {0}", btClassName);
                file.WriteLine("\t{");
                file.WriteLine("\tpublic:");
                file.WriteLine("\t\tstatic bool Create(BehaviorTree* pBT)");
            }

            file.WriteLine("\t\t{");
            file.WriteLine("\t\t\tpBT->SetClassNameString(\"BehaviorTree\");");
            file.WriteLine("\t\t\tpBT->SetId((uint16_t)-1);");
            file.WriteLine("\t\t\tpBT->SetName(\"{0}\");", filename);
            file.WriteLine("\t\t\tpBT->SetIsFSM({0});", ((Node)behavior).IsFSM ? "true" : "false");
            file.WriteLine("#if !BEHAVIAC_RELEASE");
            file.WriteLine("\t\t\tpBT->SetAgentType(\"{0}\");", agentType);
            file.WriteLine("#endif");
            if (!string.IsNullOrEmpty(((Behavior)behavior).Domains))
            {
                file.WriteLine("\t\t\tpBT->SetDomains(\"{0}\");", ((Behavior)behavior).Domains);
            }
            if (((Behavior)behavior).DescriptorRefs.Count > 0)
            {
                file.WriteLine("\t\t\tpBT->SetDescriptors(\"{0}\");", DesignerPropertyUtility.RetrieveExportValue(((Behavior)behavior).DescriptorRefs));
            }

            ExportPars(file, agentType, "pBT", (Node)behavior, "\t\t");

            // export its attachments
            ExportAttachment(file, btClassName, agentType, "pBT", (Node)behavior, "\t\t\t");

            file.WriteLine("\t\t\t// children");

            // export its children
            if (((Node)behavior).IsFSM)
            {
                file.WriteLine("\t\t\t{");
                file.WriteLine("\t\t\t\tFSM* fsm = BEHAVIAC_NEW FSM();");
                file.WriteLine("\t\t\t\tfsm->SetClassNameString(\"FSM\");");
                file.WriteLine("\t\t\t\tfsm->SetId((uint16_t)-1);");
                file.WriteLine("\t\t\t\tfsm->SetInitialId({0});", behavior.InitialStateId);
                file.WriteLine("#if !BEHAVIAC_RELEASE");
                file.WriteLine("\t\t\t\tfsm->SetAgentType(\"{0}\");", agentType);
                file.WriteLine("#endif");

                foreach (Node child in ((Node)behavior).FSMNodes)
                {
                    ExportNode(file, btClassName, agentType, "fsm", child, 4);
                }

                file.WriteLine("\t\t\t\tpBT->AddChild(fsm);");
                file.WriteLine("\t\t\t}");
            }
            else
            {
                foreach (Node child in ((Node)behavior).GetChildNodes())
                {
                    ExportNode(file, btClassName, agentType, "pBT", child, 3);
                }
            }

            file.WriteLine("\t\t\treturn true;");
            file.WriteLine("\t\t}");
            if (!onlyImplement)
            {
                file.WriteLine("\t};");
            }
            file.WriteLine();

            return new BehaviorCreator(filename, btClassName);
        }

        private void ExportTail(StreamWriter file)
        {
            file.WriteLine("\tclass CppGenerationManager : GenerationManager");
            file.WriteLine("\t{");
            file.WriteLine("\tpublic:");
            file.WriteLine("\t\tCppGenerationManager()");
            file.WriteLine("\t\t{");
            file.WriteLine("\t\t\tSetInstance(this);");
            file.WriteLine("\t\t}\n");

            file.WriteLine("\t\tvirtual void RegisterBehaviorsImplement()");
            file.WriteLine("\t\t{");
            for (int i = 0; i < _behaviorCreators.Count; ++i)
            {
                string filename = _behaviorCreators[i].Filename;
                string btClassName = _behaviorCreators[i].Classname;
                file.WriteLine("\t\t\tWorkspace::GetInstance()->RegisterBehaviorTreeCreator(\"{0}\", {1}::Create);", filename, btClassName);
            }
            file.WriteLine("\t\t}");
            file.WriteLine("\t};\n");

            file.WriteLine("\tCppGenerationManager _cppGenerationManager_;");

            // close namespace
            //file.WriteLine("}");
        }

        private void ExportPars(StreamWriter file, string agentType, string nodeName, Node node, string indent)
        {
            if (node is Behavior)
            {
                ExportPars(file, agentType, nodeName, ((Behavior)node).LocalVars, indent);
            }
        }

        private void ExportPars(StreamWriter file, string agentType, string nodeName, List<Behaviac.Design.ParInfo> pars, string indent)
        {
            if (pars.Count > 0)
            {
                file.WriteLine("{0}\t// pars", indent);
                for (int i = 0; i < pars.Count; ++i)
                {
                    string name = pars[i].BasicName;
                    string type = pars[i].NativeType;
                    string value = pars[i].DefaultValue.Replace("\"", "\\\"");

                    file.WriteLine("{0}\t{1}->AddPar(\"{2}\", \"{3}\", \"{4}\", \"{5}\");", indent, nodeName, agentType, type, name, value);
                }
            }
        }

        private void ExportAttachmentClass(StreamWriter file, string btClassName, Node node)
        {
            foreach (Behaviac.Design.Attachments.Attachment attach in node.Attachments)
            {
                if (!attach.Enable)
                    continue;

                string nodeName = string.Format("attach{0}", attach.Id);

                AttachmentCppExporter attachmentExporter = AttachmentCppExporter.CreateInstance(attach);
                attachmentExporter.GenerateClass(attach, file, "", nodeName, btClassName);
            }
        }

        private void ExportAttachment(StreamWriter file, string btClassName, string agentType, string parentName, Node node, string indent)
        {
            if (node.Attachments.Count > 0)
            {
                file.WriteLine("{0}// attachments", indent);
                foreach (Behaviac.Design.Attachments.Attachment attach in node.Attachments)
                {
                    if (!attach.Enable || attach.IsStartCondition)
                        continue;

                    file.WriteLine("{0}{{", indent);

                    string nodeName = string.Format("attach{0}", attach.Id);

                    // export its instance and the properties
                    AttachmentCppExporter attachmentExporter = AttachmentCppExporter.CreateInstance(attach);
                    attachmentExporter.GenerateInstance(attach, file, indent, nodeName, agentType, btClassName);

                    string isPrecondition = attach.IsPrecondition && !attach.IsTransition ? "true" : "false";
                    string isEffector = attach.IsEffector && !attach.IsTransition ? "true" : "false";
                    string isTransition = attach.IsTransition ? "true" : "false";
                    file.WriteLine("{0}\t{1}->Attach({2}, {3}, {4}, {5});", indent, parentName, nodeName, isPrecondition, isEffector, isTransition);
                    file.WriteLine("{0}\t{1}->SetHasEvents({1}->HasEvents() | (Event::DynamicCast({2}) != 0));", indent, parentName, nodeName);
                    file.WriteLine("{0}}}", indent);
                }
            }
        }

        private void ExportNodeClass(StreamWriter file, string btClassName, string agentType, BehaviorNode behavior, Node node)
        {
            if (!node.Enable)
                return;

            string nodeName = string.Format("node{0}", node.Id);

            NodeCppExporter nodeExporter = NodeCppExporter.CreateInstance(node);
            nodeExporter.GenerateClass(node, file, "", nodeName, agentType, btClassName);

            ExportAttachmentClass(file, btClassName, node);

            if (!(node is ReferencedBehavior))
            {
                foreach (Node child in node.GetChildNodes())
                {
                    ExportNodeClass(file, btClassName, agentType, behavior, child);
                }
            }
        }

        private void ExportNode(StreamWriter file, string btClassName, string agentType, string parentName, Node node, int indentDepth)
        {
            if (!node.Enable)
                return;

            // generate the indent string
            string indent = string.Empty;
            for (int i = 0; i < indentDepth; ++i)
            {
                indent += '\t';
            }

            string nodeName = string.Format("node{0}", node.Id);

            // open some brackets for a better formatting in the generated code
            file.WriteLine("{0}{{", indent);

            // export its instance and the properties
            NodeCppExporter nodeExporter = NodeCppExporter.CreateInstance(node);
            nodeExporter.GenerateInstance(node, file, indent, nodeName, agentType, btClassName);

            ExportPars(file, agentType, nodeName, node, indent);

            ExportAttachment(file, btClassName, agentType, nodeName, node, indent + "\t");

            bool isAsChild = true;
            if (node.Parent != null)
            {
                BaseNode.Connector connector = node.Parent.GetConnector(node);
                if (connector != null && !connector.IsAsChild)
                {
                    isAsChild = false;
                }
            }

            if (isAsChild)
            {
                // add the node to its parent
                file.WriteLine("{0}\t{1}->AddChild({2});", indent, parentName, nodeName);
            }
            else
            {
                // add the node as its customized children
                file.WriteLine("{0}\t{1}->SetCustomCondition({2});", indent, parentName, nodeName);
            }

            // export the child nodes
            if (!node.IsFSM && !(node is ReferencedBehavior))
            {
                foreach (Node child in node.GetChildNodes())
                {
                    ExportNode(file, btClassName, agentType, nodeName, child, indentDepth + 1);
                }
            }

            file.WriteLine("{0}\t{1}->SetHasEvents({1}->HasEvents() | {2}->HasEvents());", indent, parentName, nodeName);

            // close the brackets for a better formatting in the generated code
            file.WriteLine("{0}}}", indent);
        }

        private void PreExportMeta(StreamWriter file)
        {
            Dictionary<string, bool> allMethods = new Dictionary<string, bool>();

            foreach (AgentType agent in Plugin.AgentTypes)
            {
                if (agent.IsCustomized)
                    continue;

                string agentTypeName = agent.AgentTypeName;

                IList<MethodDef> methods = agent.GetMethods();
                foreach (MethodDef method in methods)
                {
                    if (!method.IsCustomized && !method.IsNamedEvent)
                    {
                        bool hasRefParam = false;

                        foreach (MethodDef.Param param in method.Params)
                        {
                            if (param.IsRef || param.IsOut || Plugin.IsRefType(param.Type))
                            {
                                hasRefParam = true;
                                break;
                            }
                        }

                        if (hasRefParam)
                        {
                            string methodFullname = method.Name.Replace("::", "_");

                            if (allMethods.ContainsKey(methodFullname))
                            {
                                continue;
                            }
                            else
                            {
                                allMethods[methodFullname] = true;
                            }

                            string methodReturnType = DataCppExporter.GetGeneratedNativeType(method.NativeReturnType);
                            string baseClass = (methodReturnType == "void") ? "CAgentMethodVoidBase" : string.Format("CAgentMethodBase<{0}>", methodReturnType);

                            // class
                            file.WriteLine("\t\t\tclass CMethod_{0} : public {1}", methodFullname, baseClass);
                            file.WriteLine("\t\t\t{");
                            string initVarsList = "";
                            bool isFirstTime = true;
                            foreach (MethodDef.Param param in method.Params)
                            {
                                if (Plugin.IsRefType(param.Type))
                                {
                                    file.WriteLine("\t\t\t\tIInstanceMember* _{0};", param.Name);
                                    if (isFirstTime)
                                    {
                                        isFirstTime = false;
                                        initVarsList += string.Format("_{0}(0) ", param.Name);
                                    }
                                    else
                                    {
                                        initVarsList += string.Format(", _{0}(0) ", param.Name);
                                    }
                                }
                                else
                                {
                                    string paramType = DataCppExporter.GetGeneratedNativeType(param.NativeType);
                                    file.WriteLine("\t\t\t\tIInstanceMember* _{0};", param.Name);
                                    if (isFirstTime)
                                    {
                                        isFirstTime = false;
                                        initVarsList += string.Format("_{0}(0) ", param.Name);
                                    }
                                    else
                                    {
                                        initVarsList += string.Format(", _{0}(0) ", param.Name);
                                    }
                                }
                            }

                            if (method.Params.Count > 0)
                            {
                                file.WriteLine();
                            }

                            // Constructors
                            file.WriteLine("\t\t\tpublic: ");
                            file.WriteLine("\t\t\t\tCMethod_{0}() : {1}", methodFullname, initVarsList);
                            file.WriteLine("\t\t\t\t{");
                            file.WriteLine("\t\t\t\t}");
                            file.WriteLine();

                            file.WriteLine("\t\t\t\tCMethod_{0}(CMethod_{0} &rhs) : {1}(rhs) , {2}", methodFullname, baseClass, initVarsList);
                            file.WriteLine("\t\t\t\t{");
                            file.WriteLine("\t\t\t\t}");
                            file.WriteLine();

                            //DeConstructor
                            file.WriteLine("\t\t\t\t~CMethod_{0}()", methodFullname);
                            file.WriteLine("\t\t\t\t{");
                            foreach (MethodDef.Param param in method.Params)
                            {
                                if (Plugin.IsRefType(param.Type))
                                {
                                    file.WriteLine("\t\t\t\t\tBEHAVIAC_DELETE _{0};", param.Name);
                                }
                                else
                                {
                                    string paramType = DataCppExporter.GetGeneratedNativeType(param.NativeType);
                                    file.WriteLine("\t\t\t\t\tBEHAVIAC_DELETE _{0};", param.Name);
                                }
                            }
                            file.WriteLine("\t\t\t\t}");
                            file.WriteLine();

                            // Clone()
                            file.WriteLine("\t\t\t\tvirtual IInstanceMember* clone()");
                            file.WriteLine("\t\t\t\t{");
                            file.WriteLine("\t\t\t\t\treturn BEHAVIAC_NEW CMethod_{0}(*this);", methodFullname);
                            file.WriteLine("\t\t\t\t}"); // Clone()
                            file.WriteLine();

                            // Load()
                            file.WriteLine("\t\t\t\tvirtual void load(const char* instance, behaviac::vector<behaviac::string>& paramStrs)");
                            file.WriteLine("\t\t\t\t{");

                            file.WriteLine("\t\t\t\t\tBEHAVIAC_ASSERT(paramStrs.size() == {0});", method.Params.Count);
                            file.WriteLine();
                            file.WriteLine("\t\t\t\t\tStringUtils::StringCopySafe(kInstanceNameMax, _instance, instance);");

                            for (int i = 0; i < method.Params.Count; ++i)
                            {
                                MethodDef.Param param = method.Params[i];
                                string paramType = DataCppExporter.GetGeneratedNativeType(param.NativeType);

                                if (Plugin.IsRefType(param.Type))
                                {
                                    if (paramType.EndsWith("*"))
                                    {
                                        paramType = paramType.Substring(0, paramType.Length - 1);
                                    }
                                }

                                file.WriteLine("\t\t\t\t\t_{0} = AgentMeta::TParseProperty<{1}>(paramStrs[{2}].c_str());", param.Name, paramType, i);
                            }

                            file.WriteLine("\t\t\t\t}"); // Load()
                            file.WriteLine();

                            // Run()
                            file.WriteLine("\t\t\t\tvirtual void run(Agent* self)");
                            file.WriteLine("\t\t\t\t{");

                            if (method.Params.Count > 0)
                            {
                                foreach (MethodDef.Param param in method.Params)
                                {
                                    file.WriteLine("\t\t\t\t\tBEHAVIAC_ASSERT(_{0} != NULL);", param.Name);
                                }

                                file.WriteLine();
                            }

                            if (!method.IsStatic)
                            {
                                file.WriteLine("\t\t\t\t\tself = Agent::GetParentAgent(self, _instance);");
                                file.WriteLine();
                            }

                            string paramValues = "";
                            string executeMethodParamValues = "";
                            string allParamTypes = "";
                            foreach (MethodDef.Param param in method.Params)
                            {
                                if (!string.IsNullOrEmpty(paramValues))
                                {
                                    paramValues += ", ";
                                }

                                if (!string.IsNullOrEmpty(executeMethodParamValues))
                                {
                                    executeMethodParamValues += ", ";
                                }

                                string paramType = DataCppExporter.GetGeneratedNativeType(param.NativeType);
                                string _tempParamType = paramType;
                                string _tempexecuteMethodParamType = _tempParamType;
                                if (paramType.EndsWith("&"))
                                {
                                    _tempParamType = paramType.Substring(0, paramType.Length - 1);
                                    _tempexecuteMethodParamType = _tempParamType;
                                }
                                else if (paramType.EndsWith("*"))
                                {
                                    _tempParamType = paramType.Substring(0, paramType.Length - 1);
                                }
                                string paramName = string.Format("*({0}*)_{1}->GetValueObject(self)", _tempParamType, param.Name);
                                string executeMethodParamName = string.Format("*({0}*)_{1}->GetValueObject(self)", _tempexecuteMethodParamType, param.Name);

                                allParamTypes += ", " + paramType;
                                paramValues += paramName;
                                executeMethodParamValues += executeMethodParamName;
                            }

                            string instanceName = method.IsStatic ? agentTypeName : string.Format("(({0})agent)", agentTypeName);

                            if (methodReturnType == "void")
                            {
                                if (method.IsPublic)
                                {
                                    file.WriteLine("\t\t\t\t\t{0}::{1}({2});", instanceName, method.BasicName, paramValues);
                                }
                                else
                                {
                                    string methodType = "";
                                    if (method.NativeReturnType.Contains("const "))
                                    {
                                        methodType = "const " + methodReturnType;
                                    }
                                    else
                                    {
                                        methodType = methodReturnType;
                                    }
                                    string retStr = string.Format("\t\t\t\t\t(({0}*)self)->_Execute_Method_<{1}METHOD_TYPE_{2}, {3}{4} >({5});", method.ClassName, getNamespace(method.ClassName), method.Name.Replace("::", "_"), methodType, allParamTypes, executeMethodParamValues);
                                    file.WriteLine(retStr);
                                }
                            }
                            else
                            {
                                if (method.IsPublic)
                                {
                                    file.WriteLine("\t\t\t\t\t_returnValue->value = {0}::{1}({2});", instanceName, method.BasicName, paramValues);
                                }
                                else
                                {
                                    string methodName = agentTypeName.Replace("::", "_") + "_" + method.BasicName.Replace("::", "_");
                                    string methodType = "";
                                    if (method.NativeReturnType.Contains("const "))
                                    {
                                        methodType = "const " + methodReturnType;
                                    }
                                    else
                                    {
                                        methodType = methodReturnType;
                                    }
                                    string retStr = string.Format("\t\t\t\t\t_returnValue->value = (({0}*)self)->_Execute_Method_<{1}METHOD_TYPE_{2}, {3}{4} >({5});", method.ClassName, getNamespace(method.ClassName), method.Name.Replace("::", "_"), methodType, allParamTypes, executeMethodParamValues);
                                    file.WriteLine(retStr);
                                }
                            }

                            file.WriteLine("\t\t\t\t}"); // Run()

                            file.WriteLine("\t\t\t};"); // end of class
                            file.WriteLine();
                        }
                    }
                }
            }
        }

        private void ExportDelegateMethod(StreamWriter file)
        {
            file.WriteLine("\t// ---------------------------------------------------------------------");
            file.WriteLine("\t// Delegate methods");
            file.WriteLine("\t// ---------------------------------------------------------------------");
            file.WriteLine();

            bool hasProperties = false;

            foreach (AgentType agent in Plugin.AgentTypes)
            {
                IList<PropertyDef> properties = agent.GetProperties();
                foreach (PropertyDef prop in properties)
                {
                    bool isMemberProp = prop.IsMember || agent.IsCustomized;

                    if (isMemberProp)
                    {
                        string bindingProperty = "";
                        string propName = prop.AgentType.AgentTypeName.Replace("::", "_") + "_" + prop.BasicName.Replace("[]", "");
                        string propFullType = DataCppExporter.GetGeneratedNativeType(prop.NativeType);
                        string propType = DataCppExporter.GetGeneratedNativeType(prop.NativeItemType);
                        string propBasicName = prop.BasicName.Replace("[]", "");
                        string propItemName = prop.BasicName;
                        if (prop.IsArrayElement)
                        {
                            propItemName = propItemName.Replace("[]", "[index]");
                        }
                        string agentTypeName = agent.AgentTypeName;

                        if (agent.IsStatic || isMemberProp && prop.IsStatic)
                        {
                            if (isMemberProp)
                            {
                                string setValue = "";
                                string getValue = "";

                                if (prop.IsPublic)
                                {
                                    setValue = string.Format("{0}::{1} = value;", agentTypeName, propItemName);
                                    getValue = string.Format("{0}::{1}", agentTypeName, propItemName);
                                }
                                else
                                {
                                    if (prop.IsCustomized)
                                    {
                                        if (prop.IsArrayElement)
                                        {
                                            setValue = string.Format("{0}::_get_{1}()[index] = value;", agentTypeName, propBasicName);
                                            getValue = string.Format("{0}::_get_{1}()[index]", agentTypeName, propBasicName);
                                        }
                                        else
                                        {
                                            setValue = string.Format("{0}::_set_{1}(value);", agentTypeName, propBasicName);
                                            getValue = string.Format("{0}::_get_{1}()", agentTypeName, propBasicName);
                                        }
                                    }
                                    else
                                    {
                                        string propValue = string.Format("(({0}*)0)->_Get_Property_<{1}PROPERTY_TYPE_{2}, {3} >()", agentTypeName, getNamespace(agentTypeName), propName, propFullType);

                                        if (prop.IsArrayElement)
                                        {
                                            setValue = string.Format("{0}[index] = value;", propValue);
                                            getValue = string.Format("{0}[index]", propValue);
                                        }
                                        else
                                        {
                                            setValue = string.Format("{0} = value;", propValue);
                                            getValue = propValue;
                                        }
                                    }
                                }

                                if (prop.IsReadonly)
                                {
                                    setValue = "";
                                }

                                string str_setter = "";
                                string str_getter = "";

                                if (prop.IsArrayElement)
                                {
                                    str_setter = string.Format("\n\tinline void Set_{0}({1} value, int index) {{ {2} }}", propName, propType, setValue);
                                    if (String.Compare(prop.NativeItemType, "bool", true) == 0)
                                    {
                                        str_getter = string.Format("\n\tinline const void* Get_{1}(int index)\n\t{{\n#if _MSC_VER\n\t\treturn {2}._Getptr();\n#else\n\t\tstatic ThreadBool buffer;\n\t\tbool b = {2};\n\t\tbuffer.set(b);\n\t\treturn buffer.value();\n#endif\n\t}}", propType, propName, getValue);
                                    }
                                    else
                                    {
                                        str_getter = string.Format("\n\tinline const void* Get_{0}(int index) {{ return &{1}; }}", propName, getValue);
                                    }
                                }
                                else
                                {
                                    str_setter = string.Format("\n\tinline void Set_{0}({1} value) {{ {2} }}", propName, propType, setValue);
                                    str_getter = string.Format("\n\tinline const void* Get_{0}() {{ return &{1}; }}", propName, getValue);
                                }

                                bindingProperty = str_setter + str_getter;
                            }
                            else
                            {
                                Debug.Check(false);
                            }
                        }
                        else
                        {
                            if (isMemberProp && !prop.IsStatic)
                            {
                                string setValue = "";
                                string getValue = "";

                                if (prop.IsPublic)
                                {
                                    setValue = string.Format("(({0}*)self)->{1} = value;", agentTypeName, propItemName);
                                    getValue = string.Format("(({0}*)self)->{1}", agentTypeName, propItemName);
                                }
                                else
                                {
                                    if (prop.IsCustomized)
                                    {
                                        if (prop.IsArrayElement)
                                        {
                                            setValue = string.Format("(({0}*)self)->_get_{1}()[index] = value;", agentTypeName, propBasicName);
                                            getValue = string.Format("(({0}*)self)->_get_{1}()[index]", agentTypeName, propBasicName);
                                        }
                                        else
                                        {
                                            setValue = string.Format("(({0}*)self)->_set_{1}(value);", agentTypeName, propBasicName);
                                            getValue = string.Format("(({0}*)self)->_get_{1}()", agentTypeName, propBasicName);
                                        }
                                    }
                                    else
                                    {
                                        string propValue = string.Format("(({0}*)self)->_Get_Property_<{1}PROPERTY_TYPE_{2}, {3} >()", agentTypeName, getNamespace(agentTypeName), propName, propFullType);
                                       
                                        if (prop.IsArrayElement)
                                        {
                                            setValue = string.Format("{0}[index] = value;", propValue);
                                            getValue = string.Format("{0}[index]", propValue);
                                        }
                                        else
                                        {
                                            setValue = string.Format("{0} = value;", propValue);
                                            getValue = propValue;
                                        }
                                    }
                                }

                                if (prop.IsReadonly)
                                {
                                    setValue = "";
                                }

                                string str_setter = "";
                                string str_getter = "";

                                if (prop.IsArrayElement)
                                {
                                    if (prop.IsProperty)
                                    {
                                        str_setter = string.Format("\n\tinline void Set_{0}(Agent* self, {1} value, int index) {{ self->SetVariable(\"{2}\",{3}u,value); }};", propName, propType, propBasicName, CRC32.CalcCRC(propBasicName));
                                        str_getter = string.Format("\n\tinline const void* Get_{1}(Agent* self, int index ){{ return &self->GetVariable<{0}>({2}u); }};", propType, propName, CRC32.CalcCRC(propBasicName));
                                    }
                                    else
                                    {
                                        str_setter = string.Format("\n\tinline void Set_{0}(Agent* self, {1} value, int index) {{ {2} }};", propName, propType, setValue);
                                        if (String.Compare(prop.NativeItemType, "bool", true) == 0)
                                        {
                                            str_getter = string.Format("\n\tinline const void* Get_{0}(Agent* self, int index)\n\t{{\n#if _MSC_VER\n\t\treturn {1}._Getptr();\n#else\n\t\tstatic ThreadBool buffer;\n\t\tbool b = {1};\n\t\tbuffer.set(b);\n\t\treturn buffer.value();\n#endif\n\t}}", propName, getValue);
                                        }
                                        else
                                        {
                                            str_getter = string.Format("\n\tinline const void* Get_{0}(Agent* self, int index) {{ return &{1}; }};", propName, getValue);
                                        }
                                    }
                                }
                                else
                                {
                                    if (prop.IsProperty)
                                    {
                                        str_setter = string.Format("\n\tinline void Set_{0}(Agent* self, {1} value) {{ self->SetVariable(\"{2}\",{3}u,value); }};", propName, propType, prop.BasicName, CRC32.CalcCRC(propBasicName));
                                        str_getter = string.Format("\n\tinline const void* Get_{1}(Agent* self){{ return &self->GetVariable<{0}>({2}u); }};", propType, propName, CRC32.CalcCRC(propBasicName));
                                    }
                                    else
                                    {
                                        str_setter = string.Format("\n\tinline void Set_{0}(Agent* self, {1} value) {{ {2} }};", propName, propType, setValue);
                                        str_getter = string.Format("\n\tinline const void* Get_{0}(Agent* self) {{ return &{1}; }};", propName, getValue);
                                    }
                                }

                                bindingProperty = str_setter + str_getter;
                            }
                        }

                        if (!string.IsNullOrEmpty(bindingProperty))
                        {
                            file.WriteLine(bindingProperty);

                            hasProperties = true;
                        }
                    }
                }

                if (hasProperties)
                {
                    file.WriteLine();
                }

                IList<MethodDef> methods = agent.GetMethods();
                foreach (MethodDef method in methods)
                {
                    bool hasRefParam = false;
                    foreach (MethodDef.Param param in method.Params)
                    {
                        if (param.IsRef || param.IsOut || Plugin.IsRefType(param.Type))
                        {
                            hasRefParam = true;
                            break;
                        }
                    }

                    string agentMethod = "";
                    string paramTypes = "";
                    string paramTypeValues = "";
                    string funcParamTypeValues = "";
                    string paramValues = "";
                    string allParamTypes = "";
                    if (method.IsNamedEvent || (!method.IsCustomized || method.AgentType.IsCustomized) && !hasRefParam)
                    {
                        foreach (MethodDef.Param param in method.Params)
                        {
                            if (!string.IsNullOrEmpty(paramTypes))
                            {
                                paramTypes += ", ";
                            }

                            if (!string.IsNullOrEmpty(paramValues))
                            {
                                paramValues += ", ";
                            }

                            if (!string.IsNullOrEmpty(funcParamTypeValues))
                            {
                                funcParamTypeValues += ", ";
                            }

                            string paramType = DataCppExporter.GetGeneratedNativeType(param.NativeType);
                            paramTypes += paramType;
                            paramTypeValues += ", " + paramType + " " + param.Name;
                            funcParamTypeValues += DataCppExporter.GetGeneratedNativeType(param.Type, param.NativeType) + " " + param.Name;
                            paramValues += param.Name;
                            //if (param.NativeType.Contains("const"))
                            //{
                            //    allParamTypes += ", const " + paramType;
                            //}
                            //else
                            //{
                            allParamTypes += ", " + paramType;
                            //}
                        }
                    }

                    string methodReturnType = DataCppExporter.GetGeneratedNativeType(method.NativeReturnType);
                    string agentTypeName = agent.AgentTypeName;
                    string methodName = agentTypeName.Replace("::", "_") + "_" + method.BasicName.Replace("::", "_");
                    if (method.IsNamedEvent)
                    {
                        if (!string.IsNullOrEmpty(paramTypes))
                            paramTypes = string.Format("<{0}>", paramTypes);

                        if (!string.IsNullOrEmpty(funcParamTypeValues))
                        {
                            funcParamTypeValues = ", " + funcParamTypeValues;
                        }

                        string str_setter = string.Format("\tinline void FunctionPointer_{0}(Agent* self{1}) {{ }} /* {2} */",
                            methodName, funcParamTypeValues, method.BasicName);

                        file.WriteLine(str_setter);
                    }
                    else if (!method.IsCustomized || method.AgentType.IsCustomized)
                    {
                        if (hasRefParam)
                        {
                            string methodFullname = method.Name.Replace("::", "_");
                            agentMethod = "";
                        }
                        else
                        {
                            if (method.IsStatic)
                            {
                                if (paramTypeValues.StartsWith(", "))
                                    paramTypeValues = paramTypeValues.Substring(2);

                                if (methodReturnType == "void")
                                {
                                    if (!string.IsNullOrEmpty(paramTypes))
                                        paramTypes = string.Format("<{0}>", paramTypes);

                                    agentMethod = string.Format("\tinline void FunctionPointer_{0}({1}) {{ {2}::{3}({4}); }}",
                                        methodName, funcParamTypeValues, agentTypeName, method.BasicName, paramValues);
                                }
                                else
                                {
                                    if (!string.IsNullOrEmpty(paramTypes))
                                        paramTypes = ", " + paramTypes;

                                    agentMethod = string.Format("\tinline {0} FunctionPointer_{1}({2}) {{ return {3}::{4}({5}); }}",
                                        methodReturnType, methodName, funcParamTypeValues, agentTypeName, method.BasicName, paramValues);
                                }
                            }
                            else
                            {
                                string methodStr = "";

                                if (method.IsPublic)
                                {
                                    methodStr = string.Format("(({0}*)self)->{1}({2})", agentTypeName, method.BasicName, paramValues);
                                }
                                else
                                {
                                    string methodType = "";
                                    if (method.NativeReturnType.Contains("const "))
                                    {
                                        methodType = "const " + methodReturnType;
                                    }
                                    else
                                    {
                                        methodType = methodReturnType;
                                    }
                                    methodStr = string.Format("(({0}*){1})->_Execute_Method_<{2}METHOD_TYPE_{3}, {4}{5} >({6})", method.ClassName, "self", getNamespace(method.ClassName), method.Name.Replace("::", "_"), methodType, allParamTypes, paramValues);
                                }

                                if (!string.IsNullOrEmpty(funcParamTypeValues))
                                {
                                    funcParamTypeValues = ", " + funcParamTypeValues;
                                }

                                if (methodReturnType == "void")
                                {
                                    agentMethod = string.Format("\tinline void FunctionPointer_{0}(Agent* self{1}) {{ {2}; }}",
                                       methodName, funcParamTypeValues, methodStr);
                                }
                                else
                                {
                                    if (!method.IsPublic)
                                        methodStr = string.Format("({0}){1}", methodReturnType, methodStr);

                                    agentMethod = string.Format("\tinline {0} FunctionPointer_{1}(Agent* self{2}) {{ return {3}; }}",
                                        methodReturnType, methodName, funcParamTypeValues, methodStr);
                                }
                            }
                        }

                        if (!string.IsNullOrEmpty(agentMethod))
                        {
                            file.WriteLine(agentMethod);
                        }
                    }
                }
            }
        }

        private void ExportAgentsDefinition(string agentFolder)
        {
            Encoding utf8WithBom = new UTF8Encoding(true);

            foreach (AgentType agent in Plugin.AgentTypes)
            {
                if (!agent.IsCustomized)
                    continue;

                string filename = Path.Combine(agentFolder, agent.BasicClassName + ".h");
                string incFilename = Path.ChangeExtension(filename, ".inc");
                if (!File.Exists(incFilename))
                {
                    using (StreamWriter file = new StreamWriter(incFilename, false, utf8WithBom))
                    {
                        file.WriteLine("// Write your member properties and methods here.");
                        file.WriteLine();

                        file.Close();
                    }
                }

                using (StreamWriter file = new StreamWriter(filename, false, utf8WithBom))
                {
                    file.WriteLine("// ---------------------------------------------------------------------");
                    file.WriteLine("// This agent file is auto-generated by behaviac designer,");
                    file.WriteLine("// so you should not modify it manually, instead of by the designer.");
                    file.WriteLine("// ---------------------------------------------------------------------");
                    file.WriteLine();

                    string headerFileMacro = string.Format("_BEHAVIAC_{0}_H_", agent.AgentTypeName.Replace("::", "_").ToUpperInvariant());

                    file.WriteLine("#ifndef {0}", headerFileMacro);
                    file.WriteLine("#define {0}", headerFileMacro);
                    file.WriteLine();

                    file.WriteLine("#include \"behaviac_agent_headers.h\"");
                    file.WriteLine();

                    string indent = "";
                    if (!string.IsNullOrEmpty(agent.Namespace))
                    {
                        indent = "\t";

                        file.WriteLine("namespace {0}", agent.Namespace);
                        file.WriteLine("{");
                    }

                    file.WriteLine("{0}class {1} : public {2}", indent, agent.BasicClassName, agent.Base.AgentTypeName);
                    file.WriteLine("{0}{{", indent);

                    file.WriteLine("{0}public:", indent);
                    file.WriteLine("{0}\t{1}();", indent, agent.BasicClassName);
                    file.WriteLine("{0}\tvirtual ~{1}();", indent, agent.BasicClassName);
                    file.WriteLine();

                    file.WriteLine("{0}\tBEHAVIAC_DECLARE_AGENTTYPE({1}, {2})", indent, agent.BasicClassName, agent.Base.AgentTypeName);
                    file.WriteLine();

                    IList<PropertyDef> properties = agent.GetProperties();

                    foreach (PropertyDef prop in properties)
                    {
                        if (prop.IsCustomized && !prop.IsPar && !prop.IsArrayElement)
                        {
                            string staticStr = prop.IsStatic ? "static " : "";
                            string propType = DataCppExporter.GetGeneratedNativeType(prop.Type);

                            if (prop.IsPublic)
                            {
                                file.WriteLine("{0}public:", indent);
                                file.WriteLine("{0}\t{1}{2} {3};", indent, staticStr, propType, prop.BasicName);
                            }
                            else
                            {
                                file.WriteLine("{0}private:", indent);
                                file.WriteLine("{0}\t{1}{2} {3};", indent, staticStr, propType, prop.BasicName);
                                file.WriteLine("{0}public:", indent);
                                file.WriteLine("{0}\tinline {1}void _set_{2}({3} value)", indent, staticStr, prop.BasicName, propType);
                                file.WriteLine("{0}\t{{", indent);
                                file.WriteLine("{0}\t\t{1} = value;", indent, prop.BasicName);
                                file.WriteLine("{0}\t}}", indent);
                                file.WriteLine("{0}\tinline {1}& _get_{2}()", indent, propType, prop.BasicName);
                                file.WriteLine("{0}\t{{", indent);
                                file.WriteLine("{0}\t\treturn {1};", indent, prop.BasicName);
                                file.WriteLine("{0}\t}}", indent);
                            }

                            file.WriteLine();
                        }
                    }

                    IList<MethodDef> methods = agent.GetMethods();

                    foreach (MethodDef method in methods)
                    {
                        if (method.IsCustomized && !method.IsNamedEvent)
                        {
                            string publicStr = method.IsPublic ? "public:" : "private:";
                            string staticStr = method.IsStatic ? "static " : "";

                            string allParams = "";
                            foreach (MethodDef.Param param in method.Params)
                            {
                                if (!string.IsNullOrEmpty(allParams))
                                    allParams += ", ";

                                allParams += DataCppExporter.GetGeneratedNativeType(param.NativeType) + " " + param.Name;
                            }

                            file.WriteLine("{0}{1}", indent, publicStr);
                            file.WriteLine("{0}\t{1}{2} {3}({4});", indent, staticStr, DataCppExporter.GetGeneratedNativeType(method.ReturnType), method.BasicName, allParams);
                            file.WriteLine();
                        }
                    }

                    incFilename = Path.GetFileName(incFilename);
                    file.WriteLine("#include \"{0}\"", incFilename);

                    //end of class
                    file.WriteLine("{0}}};", indent);

                    if (!string.IsNullOrEmpty(agent.Namespace))
                    {
                        //end of namespace
                        file.WriteLine("}");
                    }

                    file.WriteLine();
                    //file.WriteLine("BEHAVIAC_DECLARE_TYPE_VECTOR_HANDLER({0}*);", agent.AgentTypeName);
                    //file.WriteLine();

                    file.WriteLine("#endif");

                    file.Close();
                }
            }
        }

        private void ExportAgentsImplemention(string agentFolder)
        {
            string behaviacAgentDir = Path.Combine(Path.GetTempPath(), "Behaviac_Agents");
            behaviacAgentDir = Path.Combine(behaviacAgentDir, Workspace.Current.Name);
            if (!Directory.Exists(behaviacAgentDir))
            {
                Directory.CreateDirectory(behaviacAgentDir);
            }

            foreach (AgentType agent in Plugin.AgentTypes)
            {
                if (!agent.IsCustomized)
                {
                    continue;
                }

                string filename = Path.Combine(agentFolder, agent.BasicClassName + ".cpp");
                string baseFilename = Path.GetFileName(filename);
                baseFilename = Path.ChangeExtension(baseFilename, ".base.cpp");
                baseFilename = Path.Combine(behaviacAgentDir, baseFilename);

                if (!File.Exists(filename))
                {
                    ExportAgentMethod(agent, filename);

                    File.Copy(filename, baseFilename, true);
                }
                else
                {
                    string newFilename = Path.GetFileName(filename);
                    newFilename = Path.ChangeExtension(newFilename, ".new.cpp");
                    newFilename = Path.Combine(behaviacAgentDir, newFilename);

                    ExportAgentMethod(agent, newFilename);
                    Debug.Check(File.Exists(newFilename));

                    DiffFiles(baseFilename, filename, newFilename);
                }
            }
        }

        private static void ExportMethodComment(StreamWriter file, string indent)
        {
            file.WriteLine("{0}///< The following method header is generated by the designer,", indent);
            file.WriteLine("{0}///< so please be advised not to modify the prototype here,", indent);
            file.WriteLine("{0}///< but to modify it in the designer.", indent);
            file.WriteLine("{0}///< However, the method body can be modified here.", indent);
        }

        private static void ExportAgentMethod(AgentType agent, string filename)
        {
            Encoding utf8WithBom = new UTF8Encoding(true);

            using (StreamWriter file = new StreamWriter(filename, false, utf8WithBom))
            {
                file.WriteLine("// ---------------------------------------------------------------------");
                file.WriteLine("// THIS FILE IS ORIGINALLY GENERATED BY THE DESIGNER,");
                file.WriteLine("// SO PLEASE BE ADVISED NOT TO MODFIY THE PROTOTYPE OF THE METHODS!");
                file.WriteLine("// ---------------------------------------------------------------------");
                file.WriteLine();
                file.WriteLine("#include \"{0}.h\"", agent.BasicClassName);
                file.WriteLine();

                string indent = "";
                if (!string.IsNullOrEmpty(agent.Namespace))
                {
                    indent = "\t";

                    file.WriteLine("namespace {0}", agent.Namespace);
                    file.WriteLine("{");
                }

                bool hasStaticProperties = false;
                foreach (PropertyDef prop in agent.GetProperties())
                {
                    if (prop.IsStatic && prop.IsCustomized && !prop.IsPar && !prop.IsArrayElement)
                    {
                        hasStaticProperties = true;

                        string propType = DataCppExporter.GetGeneratedNativeType(prop.Type);
                        string defaultValue = DataCppExporter.GetGeneratedPropertyDefaultValue(prop, propType);
                        if (defaultValue != null)
                            defaultValue = " = " + defaultValue;
                        else
                            defaultValue = "";

                        file.WriteLine("{0}{1} {2}::{3}{4};", indent, propType, agent.BasicClassName, prop.BasicName, defaultValue);
                    }
                }

                if (hasStaticProperties)
                    file.WriteLine();

                file.WriteLine("{0}{1}::{1}()", indent, agent.BasicClassName);
                file.WriteLine("{0}{{", indent);
                foreach (PropertyDef prop in agent.GetProperties())
                {
                    if (!prop.IsStatic && prop.IsCustomized && !prop.IsPar && !prop.IsArrayElement)
                    {
                        string propType = DataCppExporter.GetGeneratedNativeType(prop.Type);
                        string defaultValue = DataCppExporter.GetGeneratedPropertyDefaultValue(prop, propType);
                        if (defaultValue != null)
                            file.WriteLine("{0}\t{1} = {2};", indent, prop.BasicName, defaultValue);
                    }
                }
                file.WriteLine("{0}}}", indent);
                file.WriteLine();

                file.WriteLine("{0}{1}::~{1}()", indent, agent.BasicClassName);
                file.WriteLine("{0}{{", indent);
                file.WriteLine("{0}}}", indent);
                file.WriteLine();

                foreach (MethodDef method in agent.GetMethods())
                {
                    if (method.IsCustomized && !method.IsNamedEvent)
                    {
                        string allParams = "";
                        foreach (MethodDef.Param param in method.Params)
                        {
                            if (!string.IsNullOrEmpty(allParams))
                                allParams += ", ";

                            allParams += DataCppExporter.GetGeneratedNativeType(param.NativeType) + " " + param.Name;
                        }

                        string returnValue = DataCppExporter.GetGeneratedDefaultValue(method.ReturnType, method.NativeReturnType);

                        ExportMethodComment(file, indent);
                        file.WriteLine("{0}{1} {2}::{3}({4})", indent, method.NativeReturnType, agent.BasicClassName, method.BasicName, allParams);
                        file.WriteLine("{0}{{", indent);
                        if (returnValue != null)
                        {
                            file.WriteLine("{0}\treturn {1};", indent, returnValue);
                        }
                        else
                        {
                            if (Plugin.IsCustomClassType(method.ReturnType))
                            {
                                string rtnTypename = DataCppExporter.GetBasicGeneratedNativeType(method.NativeReturnType);
                                rtnTypename = rtnTypename.Replace("*", "");

                                file.WriteLine("{0}\t{1} tmp;", indent, rtnTypename);
                                file.WriteLine("{0}\treturn tmp;", indent);
                            }
                        }
                        file.WriteLine("{0}}}", indent);
                        file.WriteLine();
                    }
                }

                if (!string.IsNullOrEmpty(agent.Namespace))
                {
                    //end of namespace
                    file.WriteLine("}");
                }

                file.Close();
            }
        }

        private void ExportCustomizedTypesDefinition(string agentFolder)
        {
            if (TypeManager.Instance.HasCustomizedTypes())
            {
                string filename = Path.Combine(agentFolder, "behaviac_customized_types.h");
                Encoding utf8WithBom = new UTF8Encoding(true);

                using (StreamWriter file = new StreamWriter(filename, false, utf8WithBom))
                {
                    file.WriteLine("// ---------------------------------------------------------------------");
                    file.WriteLine("// This file is auto-generated by behaviac designer, so please don't modify it by yourself!");
                    file.WriteLine("// ---------------------------------------------------------------------\n");

                    string headerFileMacro = "_BEHAVIAC_CUSTOMIZED_TYPES_H_";

                    file.WriteLine("#ifndef {0}", headerFileMacro);
                    file.WriteLine("#define {0}", headerFileMacro);
                    file.WriteLine();

                    file.WriteLine("#include \"behaviac/agent/agent.h\"\n");

                    if (TypeManager.Instance.HasCustomizedEnums())
                    {
                        file.WriteLine("// -------------------");
                        file.WriteLine("// Customized enums");
                        file.WriteLine("// -------------------");

                        for (int e = 0; e < TypeManager.Instance.Enums.Count; ++e)
                        {
                            EnumType enumType = TypeManager.Instance.Enums[e];
                            if (!enumType.IsCustomized || enumType.IsExtended)
                                continue; 
                            
                            file.WriteLine();

                            string indent = "";

                            if (!string.IsNullOrEmpty(enumType.Namespace))
                            {
                                indent = "\t";

                                file.WriteLine("namespace {0}", enumType.Namespace);
                                file.WriteLine("{");
                            }

                            file.WriteLine("{0}enum {1}", indent, enumType.Name);
                            file.WriteLine("{0}{{", indent);

                            for (int m = 0; m < enumType.Members.Count; ++m)
                            {
                                EnumType.EnumMemberType member = enumType.Members[m];

                                if (member.Value >= 0)
                                    file.WriteLine("{0}\t{1} = {2},", indent, member.Name, member.Value);
                                else
                                    file.WriteLine("{0}\t{1},", indent, member.Name);
                            }

                            file.WriteLine("{0}}};", indent);

                            if (!string.IsNullOrEmpty(enumType.Namespace))
                            {
                                file.WriteLine("}");
                            }
                        }

                        file.WriteLine();
                    }

                    if (TypeManager.Instance.HasCustomizedStructs())
                    {
                        file.WriteLine("// -------------------");
                        file.WriteLine("// Customized structs");
                        file.WriteLine("// -------------------");

                        for (int s = 0; s < TypeManager.Instance.Structs.Count; s++)
                        {
                            StructType structType = TypeManager.Instance.Structs[s];
                            if (!structType.IsCustomized || structType.IsExtended)
                                continue;
                            
                            file.WriteLine();

                            string indent = "";

                            if (!string.IsNullOrEmpty(structType.Namespace))
                            {
                                indent = "\t";

                                file.WriteLine("namespace {0}", structType.Namespace);
                                file.WriteLine("{");
                            }

                            if (string.IsNullOrEmpty(structType.BaseName))
                            {
                                file.WriteLine("{0}struct {1}", indent, structType.Name);
                            }
                            else
                            {
                                StructType baseStruct = TypeManager.Instance.FindStruct(structType.BaseName);
                                string baseName = baseStruct.Name;
                                if (baseStruct != null && baseStruct.Namespace != structType.Namespace)
                                {
                                    baseName = baseStruct.Namespace + "::" + baseName;
                                }

                                file.WriteLine("{0}struct {1} : public {2}", indent, structType.Name, baseName);
                            }
                            file.WriteLine("{0}{{", indent);

                            if (structType.Properties.Count > 0)
                            {
                                for (int m = 0; m < structType.Properties.Count; ++m)
                                {
                                    PropertyDef member = structType.Properties[m];

                                    file.WriteLine("{0}\t{1} {2};", indent, DataCppExporter.GetGeneratedNativeType(member.NativeType), member.BasicName);
                                }
                            }

                            string fullName = structType.Name;
                            if (!string.IsNullOrEmpty(structType.Namespace))
                            {
                                fullName = structType.Namespace + "::" + structType.Name;
                            }

                            file.WriteLine();
                            file.WriteLine("DECLARE_BEHAVIAC_STRUCT({0});", fullName);

                            file.WriteLine("{0}}};", indent);
                            if (!string.IsNullOrEmpty(structType.Namespace))
                            {
                                file.WriteLine("}");
                            }
                        }
                    }

                    if (TypeManager.Instance.HasCustomizedEnums())
                    {
                        file.WriteLine();

                        for (int e = 0; e < TypeManager.Instance.Enums.Count; ++e)
                        {
                            EnumType enumType = TypeManager.Instance.Enums[e];
                            if (!enumType.IsCustomized || enumType.IsExtended)
                                continue;

                            string fullName = enumType.Name;
                            if (!string.IsNullOrEmpty(enumType.Namespace))
                            {
                                fullName = enumType.Namespace + "::" + enumType.Name;
                            }

                            file.WriteLine("DECLARE_BEHAVIAC_ENUM({0}, {1});", fullName, enumType.Name);
                        }
                    }

                    if (TypeManager.Instance.HasCustomizedStructs())
                    {
                        file.WriteLine();

                        for (int s = 0; s < TypeManager.Instance.Structs.Count; s++)
                        {
                            StructType structType = TypeManager.Instance.Structs[s];
                            if (!structType.IsCustomized || structType.IsExtended)
                                continue;

                            string fullName = structType.Name;
                            if (!string.IsNullOrEmpty(structType.Namespace))
                            {
                                fullName = structType.Namespace + "::" + structType.Name;
                            }

                            file.WriteLine("BEHAVIAC_DECLARE_TYPE_VECTOR_HANDLER({0});", fullName);
                        }
                    }

                    file.WriteLine("#endif // {0}", headerFileMacro);
                }
            }
        }

        private void ExportCustomizedTypesImplemention(string agentFolder)
        {
            if (TypeManager.Instance.HasCustomizedTypes())
            {
                string filename = Path.Combine(agentFolder, "behaviac_customized_types.cpp");
                Encoding utf8WithBom = new UTF8Encoding(true);

                using (StreamWriter file = new StreamWriter(filename, false, utf8WithBom))
                {
                    file.WriteLine("// ---------------------------------------------------------------------");
                    file.WriteLine("// This file is auto-generated by behaviac designer, so please don't modify it by yourself!");
                    file.WriteLine("// ---------------------------------------------------------------------\n");

                    file.WriteLine("#include \"behaviac/agent/registermacros.h\"");
                    file.WriteLine("#include \"behaviac_customized_types.h\"\n");

                    if (TypeManager.Instance.HasCustomizedEnums())
                    {
                        file.WriteLine("// -------------------");
                        file.WriteLine("// Customized enums");
                        file.WriteLine("// -------------------");
                        file.WriteLine();

                        for (int e = 0; e < TypeManager.Instance.Enums.Count; ++e)
                        {
                            EnumType enumType = TypeManager.Instance.Enums[e];
                            if (!enumType.IsCustomized || enumType.IsExtended)
                                continue;

                            if (e > 0)
                                file.WriteLine();

                            string fullName = enumType.Name;
                            if (!string.IsNullOrEmpty(enumType.Namespace))
                            {
                                fullName = enumType.Namespace + "::" + enumType.Name;
                            }

                            file.WriteLine("BEHAVIAC_BEGIN_ENUM({0}, {1})", fullName, enumType.Name);
                            file.WriteLine("{");
                            file.WriteLine("\tBEHAVIAC_ENUMCLASS_DISPLAY_INFO(L\"{0}\", L\"{1}\");", enumType.DisplayName, enumType.Description);
                            file.WriteLine();

                            for (int m = 0; m < enumType.Members.Count; ++m)
                            {
                                EnumType.EnumMemberType member = enumType.Members[m];

                                string fullMemberName = member.Name;
                                if (!string.IsNullOrEmpty(enumType.Namespace))
                                {
                                    fullMemberName = enumType.Namespace + "::" + member.Name;
                                }

                                if (member.DisplayName != member.Name || !string.IsNullOrEmpty(member.Description))
                                    file.WriteLine("\tBEHAVIAC_ENUM_ITEM({0}, \"{1}\").DISPLAY_INFO(L\"{2}\", L\"{3}\");", fullMemberName, member.Name, member.DisplayName, member.Description);
                                else
                                    file.WriteLine("\tBEHAVIAC_ENUM_ITEM({0}, \"{1}\");", fullMemberName, member.Name);
                            }

                            file.WriteLine("}");
                            file.WriteLine("BEHAVIAC_END_ENUM()");
                        }

                        file.WriteLine();
                    }

                    if (TypeManager.Instance.HasCustomizedStructs())
                    {
                        file.WriteLine("// -------------------");
                        file.WriteLine("// Customized structs");
                        file.WriteLine("// -------------------");
                        file.WriteLine();

                        for (int s = 0; s < TypeManager.Instance.Structs.Count; s++)
                        {
                            StructType structType = TypeManager.Instance.Structs[s];
                            if (!structType.IsCustomized || structType.IsExtended)
                                continue;

                            if (s > 0)
                                file.WriteLine();

                            string fullName = structType.Name;
                            if (!string.IsNullOrEmpty(structType.Namespace))
                            {
                                fullName = structType.Namespace + "::" + structType.Name;
                            }

                            file.WriteLine("BEHAVIAC_BEGIN_STRUCT({0})", fullName);
                            file.WriteLine("{");
                            file.WriteLine("\tBEHAVIAC_STRUCT_DISPLAYNAME(L\"{0}\");", structType.DisplayName);
                            file.WriteLine("\tBEHAVIAC_STRUCT_DESC(L\"{0}\");", structType.Description);
                            file.WriteLine();

                            for (int m = 0; m < structType.Properties.Count; ++m)
                            {
                                PropertyDef member = structType.Properties[m];
                                if (member.DisplayName != member.Name || !string.IsNullOrEmpty(member.BasicDescription))
                                    file.WriteLine("\tBEHAVIAC_REGISTER_STRUCT_PROPERTY({0}).DISPLAYNAME(L\"{1}\").DESC(L\"{2}\");", member.BasicName, member.DisplayName, member.BasicDescription);
                                else
                                    file.WriteLine("\tBEHAVIAC_REGISTER_STRUCT_PROPERTY({0});", member.BasicName);
                            }

                            file.WriteLine("}");
                            file.WriteLine("BEHAVIAC_END_STRUCT()");
                        }
                    }
                }
            }
        }

        private void ExportAgentMeta(string agentFolder)
        {
            string filename = Path.Combine(agentFolder, "behaviac_agent_meta.h");
            Encoding utf8WithBom = new UTF8Encoding(true);

            using (StreamWriter file = new StreamWriter(filename, false, utf8WithBom))
            {
                file.WriteLine("// ---------------------------------------------------------------------");
                file.WriteLine("// This file is auto-generated by behaviac designer, so please don't modify it by yourself!");
                file.WriteLine("// ---------------------------------------------------------------------");
                file.WriteLine();

                string headerFileMacro = "_BEHAVIAC_AGENT_PROPERTIES_H_";

                file.WriteLine("#ifndef {0}", headerFileMacro);
                file.WriteLine("#define {0}", headerFileMacro);
                file.WriteLine();

                file.WriteLine("#include \"behaviac_agent_headers.h\"");
                file.WriteLine("#include \"behaviac_agent_member_visitor.h\"");
                if (TypeManager.Instance.HasCustomizedTypes())
                {
                    file.WriteLine("#include \"behaviac_customized_types.h\"");
                }

                file.WriteLine();
                file.WriteLine("namespace behaviac");
                file.WriteLine("{");

                ExportDelegateMethod(file);
                
                file.WriteLine("}");
                file.WriteLine("#endif // {0}", headerFileMacro);
            }

            string cppFilename = Path.Combine(agentFolder, "behaviac_agent_meta.cpp");

            using (StreamWriter file = new StreamWriter(cppFilename, false, utf8WithBom))
            {
                file.WriteLine("// ---------------------------------------------------------------------");
                file.WriteLine("// This file is auto-generated by behaviac designer, so please don't modify it by yourself!");
                file.WriteLine("// ---------------------------------------------------------------------");
                file.WriteLine();

                file.WriteLine("#include \"behaviac_agent_meta.h\"");
                file.WriteLine();
                file.WriteLine("namespace behaviac");
                file.WriteLine("{");

                file.WriteLine("\tclass CppBehaviorLoaderImplement : CppBehaviorLoader");
                file.WriteLine("\t{");

                file.WriteLine("\tpublic:");
                file.WriteLine("\t\tCppBehaviorLoaderImplement()");
                file.WriteLine("\t\t{");
                file.WriteLine("\t\t\tAgentMeta::SetInstance(this);");
                file.WriteLine("\t\t}\n");

                // destructor
                file.WriteLine("\t\tvirtual ~CppBehaviorLoaderImplement()");
                file.WriteLine("\t\t{");
                file.WriteLine("\t\t}\n");

                // load
                file.WriteLine("\t\tvirtual bool load()");
                file.WriteLine("\t\t{");

                ExportMembers(file);

                file.WriteLine();

                foreach (AgentType agentType in Plugin.AgentTypes)
                {
                    bool isStatic = (agentType != null) ? agentType.IsStatic : false;
                    if (!isStatic)
                    {
                        file.WriteLine("\t\t\tAgentMeta::Register<{0}>(\"{0}\");", agentType.AgentTypeName);
                    }
                }

                foreach (EnumType enumType in TypeManager.Instance.Enums)
                {
                    string enumFullname = enumType.Fullname;
                    file.WriteLine("\t\t\tAgentMeta::Register<{0}>(\"{0}\");", enumFullname);
                }

                foreach (StructType structType in TypeManager.Instance.Structs)
                {
                    string structFullname = structType.Fullname;
                    file.WriteLine("\t\t\tAgentMeta::Register<{0}>(\"{0}\");", structFullname);
                }

                if (Plugin.InstanceNames.Count > 0)
                {
                    file.WriteLine();

                    foreach (Plugin.InstanceName_t instance in Plugin.InstanceNames)
                    {
                        file.WriteLine("\t\t\tAgent::RegisterInstanceName<{0}>(\"{1}\");", instance.className_, instance.name_);
                    }
                }

                file.WriteLine("\n\t\t\treturn true;");
                file.WriteLine("\t\t}\n");

                // unLoad
                file.WriteLine("\t\tvirtual bool unLoad()");
                file.WriteLine("\t\t{");

                foreach (AgentType agentType in Plugin.AgentTypes)
                {
                    bool isStatic = (agentType != null) ? agentType.IsStatic : false;
                    if (!isStatic)
                    {
                        file.WriteLine("\t\t\tAgentMeta::UnRegister<{0}>(\"{0}\");", agentType.AgentTypeName);
                    }
                }

                foreach (EnumType enumType in TypeManager.Instance.Enums)
                {
                    string enumFullname = enumType.Fullname;

                    file.WriteLine("\t\t\tAgentMeta::UnRegister<{0}>(\"{0}\");", enumFullname);
                }

                foreach (StructType structType in TypeManager.Instance.Structs)
                {
                    string structFullname = structType.Fullname;

                    file.WriteLine("\t\t\tAgentMeta::UnRegister<{0}>(\"{0}\");", structFullname);
                }

                if (Plugin.InstanceNames.Count > 0)
                {
                    file.WriteLine();

                    foreach (Plugin.InstanceName_t instance in Plugin.InstanceNames)
                    {
                        file.WriteLine("\t\t\tAgent::UnRegisterInstanceName<{0}>(\"{1}\");", instance.className_, instance.name_);
                    }
                }

                file.WriteLine("\n\t\t\treturn true;");
                file.WriteLine("\t\t}");
                file.WriteLine("\t};");
                file.WriteLine();

                file.WriteLine("\tCppBehaviorLoaderImplement _cppBehaviorLoaderImplement_;");
                file.WriteLine("}");
            }
        }

        private void ExportMembers(StreamWriter file)
        {
            PreExportMeta(file);

            file.WriteLine("\t\t\tAgentMeta::SetTotalSignature({0}u);", CRC32.CalcCRC(Plugin.Signature));
            file.WriteLine();
            file.WriteLine("\t\t\tAgentMeta* meta = NULL;");
            file.WriteLine("\t\t\tBEHAVIAC_UNUSED_VAR(meta);");

            foreach (AgentType agent in Plugin.AgentTypes)
            {
                string agentTypeName = agent.AgentTypeName;
                string signature = agent.GetSignature();

                file.WriteLine("\n\t\t\t// {0}", agentTypeName);
                file.WriteLine("\t\t\tmeta = BEHAVIAC_NEW AgentMeta({0}u);", CRC32.CalcCRC(signature));
                file.WriteLine("\t\t\tAgentMeta::GetAgentMetas()[{0}u] = meta;", CRC32.CalcCRC(agentTypeName));

                IList<PropertyDef> properties = agent.GetProperties();
                foreach (PropertyDef prop in properties)
                {
                    if (!prop.IsPar)
                    {
                        string bindingProperty = "";
                        string registerName = "RegisterMemberProperty";
                        string propType = DataCppExporter.GetGeneratedNativeType(prop.NativeItemType);
                        string propItemName = prop.BasicName;
                        string propName = prop.AgentType.AgentTypeName.Replace("::", "_") + "_" + prop.BasicName.Replace("::", "_").Replace("[]", "");
                        if (Plugin.IsRefType(prop.Type) && !propType.EndsWith("*"))
                        {
                            propType += "*";
                        }
                        if (prop.IsArrayElement)
                        {
                            propItemName = propItemName.Replace("[]", "[index]");
                        }

                        bool isMemberProp = prop.IsMember || agent.IsCustomized;

                        if (agent.IsStatic || isMemberProp && prop.IsStatic)
                        {
                            if (isMemberProp)
                            {
                                if (prop.IsArrayElement)
                                {
                                    bindingProperty = string.Format("BEHAVIAC_NEW CStaticMemberArrayItemProperty< {0} >(\"{1}\", Set_{2}, Get_{2})",
                                        propType, prop.BasicName, propName);
                                }
                                else
                                {
                                    bindingProperty = string.Format("BEHAVIAC_NEW CStaticMemberProperty< {0} >(\"{1}\", Set_{2}, Get_{2})",
                                        propType, prop.BasicName, propName);
                                }
                            }
                            else
                            {
                                Debug.Check(false);
                            }
                        }
                        else
                        {
                            if (isMemberProp)
                            {
                                if (!prop.IsStatic)
                                {
                                    string setValue = "";
                                    if (!prop.IsReadonly)
                                    {
                                        setValue = string.Format("(({0})self).{1} = value;", agentTypeName, propItemName);
                                    }

                                    if (prop.IsArrayElement)
                                    {
                                        bindingProperty = string.Format("BEHAVIAC_NEW CMemberArrayItemProperty< {0} >(\"{1}\", Set_{2}, Get_{2})",
                                            propType, prop.BasicName, propName);
                                    }
                                    else
                                    {
                                        bindingProperty = string.Format("BEHAVIAC_NEW CMemberProperty< {0} >(\"{1}\", Set_{2}, Get_{2})",
                                            propType, prop.BasicName, propName);
                                    }
                                }
                            }
                            else
                            {
                                registerName = prop.IsStatic ? "RegisterStaticCustomizedProperty" : "RegisterCustomizedProperty";

                                if (prop.IsArrayElement)
                                {
                                    string propBasicName = prop.BasicName.Replace("[]", "");
                                    bindingProperty = string.Format("BEHAVIAC_NEW CCustomizedArrayItemProperty< {0} >({1}u, \"{2}\")",
                                        propType, CRC32.CalcCRC(propBasicName), propBasicName);
                                }
                                else
                                {
                                    bindingProperty = string.Format("BEHAVIAC_NEW CCustomizedProperty< {0} >({1}u, \"{2}\", \"{3}\")",
                                        propType, CRC32.CalcCRC(prop.BasicName), prop.BasicName, prop.DefaultValue);
                                }
                            }
                        }

                        if (isMemberProp)
                        {
                            file.WriteLine("\t\t\tmeta->{0}({1}u, {2});", registerName, CRC32.CalcCRC(prop.BasicName), bindingProperty);
                        }
                    }
                }

                IList<MethodDef> methods = agent.GetMethods();
                foreach (MethodDef method in methods)
                {
                    bool hasRefParam = false;
                    foreach (MethodDef.Param param in method.Params)
                    {
                        if (param.IsRef || param.IsOut || Plugin.IsRefType(param.Type))
                        {
                            hasRefParam = true;
                            break;
                        }
                    }

                    string agentMethod = "";
                    string paramTypes = "";
                    string paramTypeValues = "";
                    string paramValues = "";
                    string template_suffix = "";
                    if (method.IsNamedEvent || (!method.IsCustomized || method.AgentType.IsCustomized) && !hasRefParam)
                    {
                        foreach (MethodDef.Param param in method.Params)
                        {
                            if (!string.IsNullOrEmpty(paramTypes))
                            {
                                paramTypes += ", ";
                            }

                            if (!string.IsNullOrEmpty(paramValues))
                            {
                                paramValues += ", ";
                            }

                            string paramType = DataCppExporter.GetGeneratedNativeType(param.Type, param.NativeType);
                            paramTypes += paramType;
                            paramTypeValues += ", " + paramType + " " + param.Name;
                            paramValues += param.Name;
                        }
                    }
                    if (method.Params.Count > 0)
                    {
                        template_suffix = string.Format("_{0}", method.Params.Count);
                    }
                    
                    string methodReturnType = DataCppExporter.GetGeneratedNativeType(method.NativeReturnType);
                    string methodName = agentTypeName.Replace("::", "_") + "_" + method.BasicName.Replace("::", "_");

                    if (method.IsNamedEvent)
                    {
                        if (!string.IsNullOrEmpty(paramTypes))
                            paramTypes = string.Format("<{0}>", paramTypes);

                        agentMethod = string.Format("BEHAVIAC_NEW CAgentMethodVoid{3}{0}(FunctionPointer_{1}) /* {2} */", paramTypes, methodName, method.BasicName, template_suffix);

                        file.WriteLine("\t\t\tmeta->RegisterMethod({0}u, {1});", CRC32.CalcCRC(method.BasicName), agentMethod);
                    }
                    else if (!method.IsCustomized || method.AgentType.IsCustomized)
                    {
                        if (hasRefParam)
                        {
                            string methodFullname = method.Name.Replace("::", "_");
                            agentMethod = string.Format("BEHAVIAC_NEW CMethod_{0}()", methodFullname);
                        }
                        else
                        {
                            if (method.IsStatic)
                            {
                                if (paramTypeValues.StartsWith(", "))
                                    paramTypeValues = paramTypeValues.Substring(2);

                                if (methodReturnType == "void")
                                {
                                    if (!string.IsNullOrEmpty(paramTypes))
                                        paramTypes = string.Format("<{0}>", paramTypes);

                                    agentMethod = string.Format("BEHAVIAC_NEW CAgentStaticMethodVoid{2}{0}(FunctionPointer_{1})",
                                        paramTypes, methodName, template_suffix);
                                }
                                else
                                {
                                    if (!string.IsNullOrEmpty(paramTypes))
                                        paramTypes = ", " + paramTypes;

                                    agentMethod = string.Format("BEHAVIAC_NEW CAgentStaticMethod{3}< {0}{1} >(FunctionPointer_{2})",
                                        methodReturnType, paramTypes, methodName, template_suffix);
                                }
                            }
                            else
                            {
                                if (methodReturnType == "void")
                                {
                                    if (!string.IsNullOrEmpty(paramTypes))
                                        paramTypes = string.Format("<{0}>", paramTypes);

                                    agentMethod = string.Format("BEHAVIAC_NEW CAgentMethodVoid{2}{0}(FunctionPointer_{1})",
                                        paramTypes, methodName, template_suffix);
                                }
                                else
                                {
                                    if (!string.IsNullOrEmpty(paramTypes))
                                        paramTypes = ", " + paramTypes;
                                    agentMethod = string.Format("BEHAVIAC_NEW CAgentMethod{3}< {0}{1} >(FunctionPointer_{2})",
                                        methodReturnType, paramTypes, methodName, template_suffix);
                                }
                            }
                        }

                        file.WriteLine("\t\t\tmeta->RegisterMethod({0}u, {1});", CRC32.CalcCRC(method.BasicName), agentMethod);
                    }
                }
            }
        }

        private static string getNamespace(string className)
        {
            if (!string.IsNullOrEmpty(className))
            {
                int index = className.LastIndexOf(":");
                if (index > 1)
                    return className.Substring(0, index - 1) + "::";
            }

            return string.Empty;
        }
    }
}
