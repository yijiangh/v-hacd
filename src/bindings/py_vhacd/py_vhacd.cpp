/* Copyright (c) 2011 Khaled Mamou (kmamou at gmail dot com)
Modified by Yijiang Huang (yijiangh@mit.edu) July 2019
 All rights reserved.


 Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

 1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

 2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

 3. The names of the contributors may not be used to endorse or promote products derived from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#define _CRT_SECURE_NO_WARNINGS
#include <algorithm>
#include <assert.h>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <string.h>
#include <string>
#include <vector>

// enable when in debugging memory leaks
//#define _CRTDBG_MAP_ALLOC

// find memory leaks with the CRT library
#ifdef _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include <stdlib.h>
#endif // _CRTDBG_MAP_ALLOC

#include "VHACD.h"
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/iostream.h>

using namespace VHACD;
using namespace std;
namespace py = pybind11;

class MyCallback : public IVHACD::IUserCallback {
public:
    MyCallback(void) {}
    ~MyCallback(){};
    void Update(const double overallProgress, const double stageProgress, const double operationProgress,
        const char* const stage, const char* const operation)
    {
        cout << setfill(' ') << setw(3) << (int)(overallProgress + 0.5) << "% "
             << "[ " << stage << " " << setfill(' ') << setw(3) << (int)(stageProgress + 0.5) << "% ] "
             << operation << " " << setfill(' ') << setw(3) << (int)(operationProgress + 0.5) << "%" << endl;
    };
};

class MyLogger : public IVHACD::IUserLogger {
public:
    MyLogger(void) {}
    MyLogger(const string& fileName) { OpenFile(fileName); }
    ~MyLogger(){};
    void Log(const char* const msg)
    {
        if (m_file.is_open()) {
            m_file << msg;
            m_file.flush();
        }
    }
    void OpenFile(const string& fileName)
    {
        m_file.open(fileName.c_str());
    }

private:
    ofstream m_file;
};

struct Material {

    float m_diffuseColor[3];
    float m_ambientIntensity;
    float m_specularColor[3];
    float m_emissiveColor[3];
    float m_shininess;
    float m_transparency;
    Material(void)
    {
        m_diffuseColor[0] = 0.5f;
        m_diffuseColor[1] = 0.5f;
        m_diffuseColor[2] = 0.5f;
        m_specularColor[0] = 0.5f;
        m_specularColor[1] = 0.5f;
        m_specularColor[2] = 0.5f;
        m_ambientIntensity = 0.4f;
        m_emissiveColor[0] = 0.0f;
        m_emissiveColor[1] = 0.0f;
        m_emissiveColor[2] = 0.0f;
        m_shininess = 0.4f;
        m_transparency = 0.5f;
    };
};

struct Parameters {
    unsigned int m_oclPlatformID;
    unsigned int m_oclDeviceID;
    string m_fileNameIn;
    string m_fileNameOut;
    string m_fileNameLog;
    bool m_run;
    IVHACD::Parameters m_paramsVHACD;
    Parameters(void)
    {
        m_run = true;
        m_oclPlatformID = 0;
        m_oclDeviceID = 0;
        m_fileNameIn = "";
        m_fileNameOut = "";
        m_fileNameLog = "";
    }
};

bool LoadOFF(const string& fileName, vector<float>& points, vector<int>& triangles, IVHACD::IUserLogger& logger, bool write_log);
bool LoadOBJ(const string& fileName, vector<float>& points, vector<int>& triangles, IVHACD::IUserLogger& logger, bool write_log);
bool SaveOFF(const string& fileName, const float* const& points, const int* const& triangles, const unsigned int& nPoints,
    const unsigned int& nTriangles, IVHACD::IUserLogger& logger, bool write_log);
bool SaveVRML2(ofstream& fout, const double* const& points, const int* const& triangles, const unsigned int& nPoints,
    const unsigned int& nTriangles, const Material& material, IVHACD::IUserLogger& logger, bool write_log);
bool SaveOBJ(ofstream& fout, const double* const& points, const int* const& triangles, const unsigned int& nPoints,
    const unsigned int& nTriangles, const Material& material, IVHACD::IUserLogger& logger, int convexPart, int vertexOffset, bool write_log);
void GetFileExtension(const string& fileName, string& fileExtension);
void ComputeRandomColor(Material& mat);
int ComputeHACD(Parameters &params, const bool &verbose,
                const bool &write_output, const bool &write_log, const bool &export_separate_files,
                std::vector<std::vector<std::vector<double>>> &mesh_verts,
                std::vector<std::vector<std::vector<int>>> &mesh_faces);

PYBIND11_MODULE(py_vhacd, m) {
    m.doc() = "python wrapper for v-hacd";

    m.def("compute_convex_decomp",
    [](std::string &input, std::string &output, std::string &log,
      int resolution, double concavity,
      int planeDownsampling, int convexhullDownsampling,
      int convexhullApproximation, int maxConvexHulls,
      double alpha, double beta,
      int pca, int mode, int maxNumVerticesPerCH, double minVolumePerCH,
      bool verbose, bool export_separate_files)
    {
      // // capture standard output from ostream
      // // https://pybind11.readthedocs.io/en/stable/advanced/pycpp/utilities.html?highlight=cout
      // py::scoped_ostream_redirect stream(
      // std::cout,                               // std::ostream&
      // py::module::import("sys").attr("stdout") // Python output
      // );
      bool write_log = true;
      bool write_output = true;

      // set up parameters
      Parameters params;
      params.m_fileNameIn = input;
      if (output.empty()) {
        write_output = false;
      } else {
        params.m_fileNameOut = output;
      }
      if (log.empty()) {
        write_log = false;
      } else {
        params.m_fileNameLog = log;
      }
      params.m_paramsVHACD.m_resolution = resolution;
      params.m_paramsVHACD.m_concavity = concavity;
      params.m_paramsVHACD.m_planeDownsampling = planeDownsampling;
      params.m_paramsVHACD.m_convexhullDownsampling = convexhullDownsampling;
      params.m_paramsVHACD.m_alpha = alpha;
      params.m_paramsVHACD.m_beta = beta;
      params.m_paramsVHACD.m_maxConvexHulls = maxConvexHulls;
      params.m_paramsVHACD.m_pca = pca;
      params.m_paramsVHACD.m_mode = mode;
      params.m_paramsVHACD.m_maxNumVerticesPerCH = maxNumVerticesPerCH;
      params.m_paramsVHACD.m_minVolumePerCH = minVolumePerCH;
      params.m_paramsVHACD.m_convexhullApproximation = convexhullApproximation;
      // params.m_paramsVHACD.m_oclAcceleration = atoi(argv[i]);
      // params.m_oclPlatformID = atoi(argv[i]);
      // params.m_oclDeviceID = atoi(argv[i]);

      params.m_paramsVHACD.m_resolution = (params.m_paramsVHACD.m_resolution < 64) ? 0 : params.m_paramsVHACD.m_resolution;
      params.m_paramsVHACD.m_planeDownsampling = (params.m_paramsVHACD.m_planeDownsampling < 1) ? 1 : params.m_paramsVHACD.m_planeDownsampling;
      params.m_paramsVHACD.m_convexhullDownsampling = (params.m_paramsVHACD.m_convexhullDownsampling < 1) ? 1 : params.m_paramsVHACD.m_convexhullDownsampling;

      std::vector<std::vector<std::vector<double>>> mesh_verts;
      std::vector<std::vector<std::vector<int>>> mesh_faces;

      int success = ComputeHACD(params, verbose, write_output, write_log, export_separate_files, mesh_verts, mesh_faces);

      if (success != 0) cout << "HCACD failed: return code: " << success << endl;

      return std::make_tuple(success, mesh_verts, mesh_faces);
    },
    py::arg("input"), py::arg("output")="", py::arg("log")="",
    py::arg("resolution")=100000, py::arg("concavity")=0.001,
    py::arg("planeDownsampling")=4, py::arg("convexhullDownsampling")=4,
    py::arg("convexhullApproximation")=1, py::arg("maxConvexHulls")=1024,
    py::arg("alpha")=0.05, py::arg("beta")=0.05,
    py::arg("pca")=0, py::arg("mode")=0,
    py::arg("maxNumVerticesPerCH")=64, py::arg("minVolumePerCH")=0.0001,
    py::arg("verbose")=0, py::arg("export_separate_files")=1
    ); // end compute function

    py::add_ostream_redirect(m, "ostream_redirect");

} // end py_vhacd def

int ComputeHACD(Parameters &params, const bool &verbose,
                const bool &write_output, const bool &write_log, const bool &export_separate_files,
                std::vector<std::vector<std::vector<double>>> &mesh_verts,
                std::vector<std::vector<std::vector<int>>> &mesh_faces)
{
  {
      MyCallback myCallback;
      MyLogger myLogger(params.m_fileNameLog);
      if (verbose) params.m_paramsVHACD.m_callback = &myCallback;
      if (write_log) params.m_paramsVHACD.m_logger = &myLogger;

      std::ostringstream msg;
      msg << "+ Parameters" << std::endl;
      msg << "\t input                                       " << params.m_fileNameIn << endl;
      msg << "\t resolution                                  " << params.m_paramsVHACD.m_resolution << endl;
      msg << "\t max. concavity                              " << params.m_paramsVHACD.m_concavity << endl;
      msg << "\t plane down-sampling                         " << params.m_paramsVHACD.m_planeDownsampling << endl;
      msg << "\t convex-hull down-sampling                   " << params.m_paramsVHACD.m_convexhullDownsampling << endl;
      msg << "\t alpha                                       " << params.m_paramsVHACD.m_alpha << endl;
      msg << "\t beta                                        " << params.m_paramsVHACD.m_beta << endl;
      msg << "\t maxhulls                                    " << params.m_paramsVHACD.m_maxConvexHulls << endl;
      msg << "\t pca                                         " << params.m_paramsVHACD.m_pca << endl;
      msg << "\t mode                                        " << params.m_paramsVHACD.m_mode << endl;
      msg << "\t max. vertices per convex-hull               " << params.m_paramsVHACD.m_maxNumVerticesPerCH << endl;
      msg << "\t min. volume to add vertices to convex-hulls " << params.m_paramsVHACD.m_minVolumePerCH << endl;
      msg << "\t convex-hull approximation                   " << params.m_paramsVHACD.m_convexhullApproximation << endl;
      msg << "\t OpenCL acceleration                         " << params.m_paramsVHACD.m_oclAcceleration << endl;
      msg << "\t OpenCL platform ID                          " << params.m_oclPlatformID << endl;
      msg << "\t OpenCL device ID                            " << params.m_oclDeviceID << endl;
      if (write_output) {
        msg << "\t output                                      " << params.m_fileNameOut << endl;
      } else {
        msg << "\t NO WRITTEN OUTPUT." << endl;
      }
      if (write_log) {
        msg << "\t log                                         " << params.m_fileNameLog << endl;
      } else {
        msg << "\t NO WRITTEN LOG." << endl;
      }
      msg << "+ Load mesh" << std::endl;
      if (write_log) myLogger.Log(msg.str().c_str());
      if (verbose) cout << msg.str().c_str();

      // load mesh
      vector<float> points;
      vector<int> triangles;
      string fileExtension;
      GetFileExtension(params.m_fileNameIn, fileExtension);
      if (fileExtension == ".OFF") {
          if (!LoadOFF(params.m_fileNameIn, points, triangles, myLogger, write_log)) {
              std::cerr << "load off fails: " << params.m_fileNameIn << endl;
              return -1;
          }
      }
      else if (fileExtension == ".OBJ") {
          if (!LoadOBJ(params.m_fileNameIn, points, triangles, myLogger, write_log)) {
              std::cerr << "load obj fails: " << params.m_fileNameIn << endl;
              return -1;
          }
      }
      else {
          if (write_log) myLogger.Log("Format not supported!\n");
          std::cerr << "input format not supported: " << params.m_fileNameIn << endl;
          return -1;
      }

      // run V-HACD
      IVHACD* interfaceVHACD = CreateVHACD();
      // IVHACD* interfaceVHACD = CreateVHACD_ASYNC();

      bool res = interfaceVHACD->Compute(&points[0], (unsigned int)points.size() / 3,
          (const uint32_t *)&triangles[0], (unsigned int)triangles.size() / 3, params.m_paramsVHACD);

      if (res) {
          if (verbose) cout << "VHACD computation succeed!" << endl;

          // generating vertices & faces for pybinding output
          unsigned int nConvexHulls = interfaceVHACD->GetNConvexHulls();
          if (write_log) myLogger.Log(msg.str().c_str());
          if (verbose) cout << "VHACD finished, # decomposed convex hulls: " << nConvexHulls << endl;

          IVHACD::ConvexHull ch;
          mesh_verts.reserve(nConvexHulls);
          mesh_faces.reserve(nConvexHulls);

          for (unsigned int p = 0; p < nConvexHulls; ++p) {
            interfaceVHACD->GetConvexHull(p, ch);

            if (verbose) cout << "fetching result convex hull #" << p
                              << ", #V: " << ch.m_nPoints << ", #F: " << ch.m_nTriangles << endl;

            std::vector<std::vector<double>> unit_m_verts;
            std::vector<std::vector<int>> unit_m_faces;
            unit_m_verts.reserve(ch.m_nPoints);
            unit_m_faces.reserve(ch.m_nTriangles);

            size_t nV = ch.m_nPoints * 3;
            size_t nT = ch.m_nTriangles * 3;

            if (nV > 0) {
                for (size_t v = 0; v < nV; v += 3) {
                    // fout << "v " << points[v + 0] << " " << points[v + 1] << " " << points[v + 2] << std::endl;
                    std::vector<double> vert;
                    vert.reserve(3);
                    vert.push_back(ch.m_points[v + 0]);
                    vert.push_back(ch.m_points[v + 1]);
                    vert.push_back(ch.m_points[v + 2]);
                    unit_m_verts.push_back(vert);
                }
            }
            if (nT > 0) {
                for (size_t f = 0; f < nT; f += 3) {
                         // fout << "f "
                         // << triangles[f + 0]+vertexOffset << " "
                         // << triangles[f + 1]+vertexOffset << " "
                         // << triangles[f + 2]+vertexOffset << " " << std::endl;
                    auto triangles = (const int *)ch.m_triangles;
                    std::vector<int> face;
                    face.reserve(3);
                    face.push_back(triangles[f + 0]);
                    face.push_back(triangles[f + 1]);
                    face.push_back(triangles[f + 2]);
                    unit_m_faces.push_back(face);
                }
            }

            mesh_verts.push_back(unit_m_verts);
            mesh_faces.push_back(unit_m_faces);
          } // end loop convex hulls

          if (write_output)
          {
            std::string ext;
            std::string output_filename;
            if (params.m_fileNameOut.length() > 4) {
              output_filename = params.m_fileNameOut.substr(0, params.m_fileNameOut.length()-4);
              ext = params.m_fileNameOut.substr(params.m_fileNameOut.length()-4);
            }
            std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
            unsigned int nConvexHulls = interfaceVHACD->GetNConvexHulls();
            if (write_log) {
              msg.str("");
              msg << "+ Generate output: " << nConvexHulls << " convex-hulls " << endl;
              myLogger.Log(msg.str().c_str());
            }
            if (verbose) {
              std::cout << "Generate output: " << nConvexHulls << " convex-hulls " << std::endl;
            }

            IVHACD::ConvexHull ch;
            if (ext == ".wrl") {
                // save output as wrl
              if (!export_separate_files) {
                ofstream foutCH(params.m_fileNameOut.c_str());
                if (foutCH.is_open()) {
                    Material mat;
                    for (unsigned int p = 0; p < nConvexHulls; ++p) {
                        interfaceVHACD->GetConvexHull(p, ch);
                        ComputeRandomColor(mat);
                        SaveVRML2(foutCH, ch.m_points, (const int *)ch.m_triangles, ch.m_nPoints, ch.m_nTriangles, mat, myLogger, write_log);
                        msg.str("");
                        msg << "\t CH[" << setfill('0') << setw(5) << p << "] " << ch.m_nPoints << " V, " << ch.m_nTriangles << " T" << endl;
                        if (write_log) myLogger.Log(msg.str().c_str());
                    }
                    foutCH.close();
                } // end foutCH isopen
              } else {
                    Material mat;
                    for (unsigned int p = 0; p < nConvexHulls; ++p) {
                        ofstream foutCH((output_filename + '_' + std::to_string(p) + ext).c_str());
                        if (foutCH.is_open()) {
                          interfaceVHACD->GetConvexHull(p, ch);
                          ComputeRandomColor(mat);
                          SaveVRML2(foutCH, ch.m_points, (const int *)ch.m_triangles, ch.m_nPoints, ch.m_nTriangles, mat, myLogger, write_log);
                          msg.str("");
                          msg << "\t CH[" << setfill('0') << setw(5) << p << "] " << ch.m_nPoints << " V, " << ch.m_nTriangles << " T" << endl;
                          if (write_log) myLogger.Log(msg.str().c_str());
                          foutCH.close();
                      } // end fout isopen
                    } // end for nConvexHulls
                } // end if export_separate_files
            } // end ext == wrl

            if (ext == ".obj") {
                // save as obj
                if (!export_separate_files) {
                  ofstream foutCH(params.m_fileNameOut.c_str());
                  if (foutCH.is_open()) {
                      Material mat;
                      int vertexOffset = 1; // obj wavefront starts counting at 1...
                      for (unsigned int p = 0; p < nConvexHulls; ++p) {
                          interfaceVHACD->GetConvexHull(p, ch);
                          SaveOBJ(foutCH, ch.m_points, (const int *)ch.m_triangles, ch.m_nPoints, ch.m_nTriangles, mat, myLogger, p, vertexOffset, write_log);
                          vertexOffset+=ch.m_nPoints;
                          msg.str("");
                          msg << "\t CH[" << setfill('0') << setw(5) << p << "] " << ch.m_nPoints << " V, " << ch.m_nTriangles << " T" << endl;
                          if (write_log) myLogger.Log(msg.str().c_str());
                      }
                      foutCH.close();
                  } // end if foutCH isopen
                }
                else {
                    Material mat;
                    for (unsigned int p = 0; p < nConvexHulls; ++p) {
                          ofstream foutCH((output_filename + '_' + std::to_string(p) + ext).c_str());
                          int vertexOffset = 1; // obj wavefront starts counting at 1...
                          if (foutCH.is_open()) {
                            interfaceVHACD->GetConvexHull(p, ch);
                            SaveOBJ(foutCH, ch.m_points, (const int *)ch.m_triangles, ch.m_nPoints, ch.m_nTriangles, mat, myLogger, p, vertexOffset, write_log);
                            msg.str("");
                            msg << "\t CH[" << setfill('0') << setw(5) << p << "] " << ch.m_nPoints << " V, " << ch.m_nTriangles << " T" << endl;
                            if (write_log) myLogger.Log(msg.str().c_str());
                            foutCH.close();
                          } // end foutCH isopen
                    } // end for nConvexHulls
                } // end if export_separate_files
            } // ext == obj

            // if (ext == ".off") {
            //     // save as off
            //     if (!export_separate_files) {
            //         Material mat;
            //         for (unsigned int p = 0; p < nConvexHulls; ++p) {
            //             interfaceVHACD->GetConvexHull(p, ch);
            //             SaveOFF(params.m_fileNameOut, ch.m_points, (const int *)ch.m_triangles, ch.m_nPoints, ch.m_nTriangles, myLogger, write_log);
            //             msg.str("");
            //             msg << "\t CH[" << setfill('0') << setw(5) << p << "] " << ch.m_nPoints << " V, " << ch.m_nTriangles << " T" << endl;
            //             if (write_log) myLogger.Log(msg.str().c_str());
            //           }
            //     } else {
            //         Material mat;
            //         for (unsigned int p = 0; p < nConvexHulls; ++p) {
            //             interfaceVHACD->GetConvexHull(p, ch);
            //             SaveOFF(output_filename + '_' + std::to_string(p) + ext, ch.m_points, (const int *)ch.m_triangles, ch.m_nPoints, ch.m_nTriangles, myLogger, write_log);
            //             msg.str("");
            //             msg << "\t CH[" << setfill('0') << setw(5) << p << "] " << ch.m_nPoints << " V, " << ch.m_nTriangles << " T" << endl;
            //             if (write_log) myLogger.Log(msg.str().c_str());
            //         } // end for nConvexHulls
            //     } // end if export_separate_files
            // } // ext == off

        } // end if write_output

      } // if res
      else {
          if(write_log) myLogger.Log("Decomposition cancelled by user!\n");
          std::cerr << "Decomposition cancelled by user!\n" << endl;
      }

      interfaceVHACD->Clean();
      interfaceVHACD->Release();

      if (verbose) cout << "VHACD clean, release finished" << endl;
    } // end main program

#ifdef _CRTDBG_MAP_ALLOC
    _CrtDumpMemoryLeaks();
#endif // _CRTDBG_MAP_ALLOC
    return 0;
}

void GetFileExtension(const string& fileName, string& fileExtension)
{
    size_t lastDotPosition = fileName.find_last_of(".");
    if (lastDotPosition == string::npos) {
        fileExtension = "";
    }
    else {
        fileExtension = fileName.substr(lastDotPosition, fileName.size());
        transform(fileExtension.begin(), fileExtension.end(), fileExtension.begin(), ::toupper);
    }
}

void ComputeRandomColor(Material& mat)
{
    mat.m_diffuseColor[0] = mat.m_diffuseColor[1] = mat.m_diffuseColor[2] = 0.0f;
    while (mat.m_diffuseColor[0] == mat.m_diffuseColor[1] || mat.m_diffuseColor[2] == mat.m_diffuseColor[1] || mat.m_diffuseColor[2] == mat.m_diffuseColor[0]) {
        mat.m_diffuseColor[0] = (rand() % 100) / 100.0f;
        mat.m_diffuseColor[1] = (rand() % 100) / 100.0f;
        mat.m_diffuseColor[2] = (rand() % 100) / 100.0f;
    }
}

bool LoadOFF(const string& fileName, vector<float>& points, vector<int>& triangles, IVHACD::IUserLogger& logger, bool write_log)
{
    FILE* fid = fopen(fileName.c_str(), "r");
    if (fid) {
        const string strOFF("OFF");
        char temp[1024];
        fscanf(fid, "%s", temp);
        if (string(temp) != strOFF) {
            if (write_log) logger.Log("Loading error: format not recognized \n");
            std::cerr << "Loading OFF file error: format not recognized" << std::endl;
            fclose(fid);
            return false;
        }
        else {
            int nv = 0;
            int nf = 0;
            int ne = 0;
            fscanf(fid, "%i", &nv);
            fscanf(fid, "%i", &nf);
            fscanf(fid, "%i", &ne);
            points.resize(nv * 3);
            triangles.resize(nf * 3);
            const int np = nv * 3;
            for (int p = 0; p < np; p++) {
                fscanf(fid, "%f", &(points[p]));
            }
            int s;
            for (int t = 0, r = 0; t < nf; ++t) {
                fscanf(fid, "%i", &s);
                if (s == 3) {
                    fscanf(fid, "%i", &(triangles[r++]));
                    fscanf(fid, "%i", &(triangles[r++]));
                    fscanf(fid, "%i", &(triangles[r++]));
                }
                else // Fix me: support only triangular meshes
                {
                    for (int h = 0; h < s; ++h)
                        fscanf(fid, "%i", &s);
                }
            }
            fclose(fid);
        }
    }
    else {
        if (write_log) logger.Log("Loading error: file not found \n");
        std::cerr << "LoadOFF: file not found" << std::endl;
        return false;
    }
    return true;
}

bool LoadOBJ(const string& fileName, vector<float>& points, vector<int>& triangles, IVHACD::IUserLogger& logger, bool write_log)
{
    const unsigned int BufferSize = 1024;
    FILE* fid = fopen(fileName.c_str(), "r");

    if (fid) {
        char buffer[BufferSize];
        int ip[4];
        float x[3];
        char* pch;
        char* str;
        while (!feof(fid)) {
            if (!fgets(buffer, BufferSize, fid)) {
                break;
            }
            else if (buffer[0] == 'v') {
                if (buffer[1] == ' ') {
                    str = buffer + 2;
                    for (int k = 0; k < 3; ++k) {
                        pch = strtok(str, " ");
                        if (pch)
                            x[k] = (float)atof(pch);
                        else {
                            return false;
                        }
                        str = NULL;
                    }
                    points.push_back(x[0]);
                    points.push_back(x[1]);
                    points.push_back(x[2]);
                }
            }
            else if (buffer[0] == 'f') {

                pch = str = buffer + 2;
                int k = 0;
                while (pch) {
                    pch = strtok(str, " ");
                    if (pch && *pch != '\n') {
                        ip[k++] = atoi(pch) - 1;
                    }
                    else {
                        break;
                    }
                    str = NULL;
                }
                if (k == 3) {
                    triangles.push_back(ip[0]);
                    triangles.push_back(ip[1]);
                    triangles.push_back(ip[2]);
                }
                else if (k == 4) {
                    triangles.push_back(ip[0]);
                    triangles.push_back(ip[1]);
                    triangles.push_back(ip[2]);

                    triangles.push_back(ip[0]);
                    triangles.push_back(ip[2]);
                    triangles.push_back(ip[3]);
                }
            }
        }
        fclose(fid);
    }
    else {
        if (write_log) logger.Log("File not found\n");
        std::cerr << "LoadOBJ: cannot open file!" << std::endl;
        return false;
    }
    return true;
}


bool SaveOFF(const string& fileName, const float* const& points, const int* const& triangles, const unsigned int& nPoints,
    const unsigned int& nTriangles, IVHACD::IUserLogger& logger, bool write_log)
{
    ofstream fout(fileName.c_str());
    if (fout.is_open()) {
        size_t nV = nPoints * 3;
        size_t nT = nTriangles * 3;
        fout << "OFF" << std::endl;
        fout << nPoints << " " << nTriangles << " " << 0 << std::endl;
        for (size_t v = 0; v < nV; v += 3) {
            fout << points[v + 0] << " "
                 << points[v + 1] << " "
                 << points[v + 2] << std::endl;
        }
        for (size_t f = 0; f < nT; f += 3) {
            fout << "3 " << triangles[f + 0] << " "
                 << triangles[f + 1] << " "
                 << triangles[f + 2] << std::endl;
        }
        fout.close();
        return true;
    }
    else {
        if (write_log) logger.Log("Can't open file\n");
        std::cerr << "SaveOFF: cannot open file!" << std::endl;
        return false;
    }
}


bool SaveVRML2(ofstream& fout, const double* const& points, const int* const& triangles, const unsigned int& nPoints,
    const unsigned int& nTriangles, const Material& material, IVHACD::IUserLogger& logger, bool write_log)
{
    if (fout.is_open()) {
        fout.setf(std::ios::fixed, std::ios::floatfield);
        fout.setf(std::ios::showpoint);
        fout.precision(6);
        size_t nV = nPoints * 3;
        size_t nT = nTriangles * 3;
        fout << "#VRML V2.0 utf8" << std::endl;
        fout << "" << std::endl;
        fout << "# Vertices: " << nPoints << std::endl;
        fout << "# Triangles: " << nTriangles << std::endl;
        fout << "" << std::endl;
        fout << "Group {" << std::endl;
        fout << "    children [" << std::endl;
        fout << "        Shape {" << std::endl;
        fout << "            appearance Appearance {" << std::endl;
        fout << "                material Material {" << std::endl;
        fout << "                    diffuseColor " << material.m_diffuseColor[0] << " "
             << material.m_diffuseColor[1] << " "
             << material.m_diffuseColor[2] << std::endl;
        fout << "                    ambientIntensity " << material.m_ambientIntensity << std::endl;
        fout << "                    specularColor " << material.m_specularColor[0] << " "
             << material.m_specularColor[1] << " "
             << material.m_specularColor[2] << std::endl;
        fout << "                    emissiveColor " << material.m_emissiveColor[0] << " "
             << material.m_emissiveColor[1] << " "
             << material.m_emissiveColor[2] << std::endl;
        fout << "                    shininess " << material.m_shininess << std::endl;
        fout << "                    transparency " << material.m_transparency << std::endl;
        fout << "                }" << std::endl;
        fout << "            }" << std::endl;
        fout << "            geometry IndexedFaceSet {" << std::endl;
        fout << "                ccw TRUE" << std::endl;
        fout << "                solid TRUE" << std::endl;
        fout << "                convex TRUE" << std::endl;
        if (nV > 0) {
            fout << "                coord DEF co Coordinate {" << std::endl;
            fout << "                    point [" << std::endl;
            for (size_t v = 0; v < nV; v += 3) {
                fout << "                        " << points[v + 0] << " "
                     << points[v + 1] << " "
                     << points[v + 2] << "," << std::endl;
            }
            fout << "                    ]" << std::endl;
            fout << "                }" << std::endl;
        }
        if (nT > 0) {
            fout << "                coordIndex [ " << std::endl;
            for (size_t f = 0; f < nT; f += 3) {
                fout << "                        " << triangles[f + 0] << ", "
                     << triangles[f + 1] << ", "
                     << triangles[f + 2] << ", -1," << std::endl;
            }
            fout << "                ]" << std::endl;
        }
        fout << "            }" << std::endl;
        fout << "        }" << std::endl;
        fout << "    ]" << std::endl;
        fout << "}" << std::endl;
        return true;
    }
    else {
        if (write_log) logger.Log("Can't open file\n");
        std::cerr << "SaveVRML2: cannot open file!" << std::endl;
        return false;
    }
}


bool SaveOBJ(ofstream& fout, const double* const& points, const int* const& triangles, const unsigned int& nPoints,
    const unsigned int& nTriangles, const Material& material, IVHACD::IUserLogger& logger, int convexPart, int vertexOffset, bool write_log)
{
    if (fout.is_open()) {

        fout.setf(std::ios::fixed, std::ios::floatfield);
        fout.setf(std::ios::showpoint);
        fout.precision(6);
        size_t nV = nPoints * 3;
        size_t nT = nTriangles * 3;

	      fout << "o convex_" << convexPart << std::endl;

        if (nV > 0) {
            for (size_t v = 0; v < nV; v += 3) {
                fout << "v " << points[v + 0] << " " << points[v + 1] << " " << points[v + 2] << std::endl;
            }
        }
        if (nT > 0) {
            for (size_t f = 0; f < nT; f += 3) {
                     fout << "f "
                     << triangles[f + 0]+vertexOffset << " "
                     << triangles[f + 1]+vertexOffset << " "
                     << triangles[f + 2]+vertexOffset << " " << std::endl;
            }
        }
        return true;
    }
    else {
        if (write_log) logger.Log("Can't open file\n");
        std::cerr << "SaveOBJ: cannot open file" << std::endl;
        return false;
    }
}
