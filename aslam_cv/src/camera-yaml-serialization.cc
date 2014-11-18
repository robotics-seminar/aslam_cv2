#include <aslam/cameras/camera.h>
#include <aslam/cameras/camera-pinhole.h>
#include <aslam/cameras/camera-unified-projection.h>
#include <aslam/cameras/camera-yaml-serialization.h>
#include <aslam/cameras/distortion-equidistant.h>
#include <aslam/cameras/distortion-fisheye.h>
#include <aslam/cameras/distortion-radtan.h>
#include <aslam/common/yaml-serialization.h>

namespace YAML {

bool convert<std::shared_ptr<aslam::Camera> >::decode(
    const Node& node, std::shared_ptr<aslam::Camera>& camera) {
  camera.reset();
  try {
    if(!node.IsMap()) {
      LOG(ERROR) << "Unable to get parse the camera because the node is not a map.";
      return true;
    }
    // Determine the distortion type. Start with no distortion.
    aslam::Distortion::UniquePtr distortion;
    const YAML::Node distortion_config = node["distortion"];

    if(distortion_config) {
      std::string distortion_type;
      Eigen::VectorXd distortion_parameters;
      if(YAML::safeGet(distortion_config, "type", &distortion_type) &&
         YAML::safeGet(distortion_config, "parameters", &distortion_parameters)) {
        if(distortion_type == "none") {
            distortion = nullptr;
        } else if(distortion_type == "equidistant") {
          distortion.reset(new aslam::EquidistantDistortion(distortion_parameters));
        } else if(distortion_type == "fisheye") {
          distortion.reset(new aslam::FisheyeDistortion(distortion_parameters));
        } else if(distortion_type == "radial-tangential") {
            distortion.reset(new aslam::RadTanDistortion(distortion_parameters));
        } else {
            LOG(FATAL) << "Unknown distortion model: \"" << distortion_type << "\". "
                << "Valid values are {none, equidistant, fisheye, radial-tangential}.";
            return true;
        }
        if (distortion != nullptr &&
            !distortion->distortionParametersValid(distortion_parameters)) {
          LOG(ERROR) << "Invalid distortion parameters: " << distortion_parameters.transpose();
          return true;
        }
      } else {
        LOG(ERROR) << "Unable to get the required parameters from the distortion. "
            << "Required: string type, VectorXd parameters.";
        return true;
      }
    } else {
      LOG(INFO) << "Found a camera with no distortion.";
    }

    std::string camera_type;
    unsigned image_width;
    unsigned image_height;
    Eigen::VectorXd intrinsics;
    if(YAML::safeGet(node, "type", &camera_type) &&
        YAML::safeGet(node, "image_width", &image_width) &&
        YAML::safeGet(node, "image_height", &image_height) &&
        YAML::safeGet(node, "intrinsics", &intrinsics)){
      if(camera_type == "pinhole") {
        if(intrinsics.size() != aslam::PinholeCamera::parameterCount()) {
          LOG(ERROR) << "Wrong number of intrinsic parameters for the pinhole camera. "
              << "Wanted: " << aslam::PinholeCamera::parameterCount() << ", got: "
              << intrinsics.size();
          return true;
        }
        camera.reset(new aslam::PinholeCamera(intrinsics, image_width, image_height,
                                       distortion));
      } else if(camera_type == "unified-projection") {
        if(intrinsics.size() != aslam::UnifiedProjectionCamera::parameterCount()) {
          LOG(ERROR) << "Wrong number of intrinsic parameters for the unified projection camera. "
              << "Wanted: " << aslam::UnifiedProjectionCamera::parameterCount() << ", got: "
              << intrinsics.size();
          return true;
        }
        camera.reset(new aslam::UnifiedProjectionCamera(intrinsics, image_width,
                                                 image_height, distortion));
      } else {
        LOG(ERROR) << "Unknown camera model: \"" << camera_type << "\". "
            << "Valid values are {pinhole, unified-projection}.";
        return true;
      }
    } else {
      LOG(ERROR) << "Unable to get the required parameters from the camera. "
          << "Required: string type, int image_height, int image_width, VectorXd intrinsics.";
      return true;
    }
    // ID
    if(node["id"]) {
      aslam::CameraId id;
      std::string id_string = node["id"].as<std::string>();
      if(id.fromHexString(id_string)) {
        camera->setId(id);
      } else {
        LOG(ERROR) << "Unable to parse \"" << id_string << "\" as a hex string.";
        camera.reset();
        return true;
      }
    }

    if(node["line-delay-nanoseconds"]) {
      uint64_t line_delay_nanoseconds = camera->getLineDelayNanoSeconds();
      if(YAML::safeGet(node, "line-delay-nanoseconds", &line_delay_nanoseconds)){
        camera->setLineDelayNanoSeconds(line_delay_nanoseconds);
      } else {
        LOG(ERROR) << "Unable to parse the parameter line-delay-nanoseconds.";
        camera.reset();
        return true;
      }
    }

    if(node["label"]) {
      camera->setLabel(node["label"].as<std::string>());
    }
  } catch(const std::exception& e) {
    camera = nullptr;
    LOG(ERROR) << "Yaml exception during parsing: " << e.what();
    camera.reset();
    return true;
  }
  return true;
}

Node convert<std::shared_ptr<aslam::Camera> >::encode(
    const std::shared_ptr<aslam::Camera>& camera) {
  CHECK_NOTNULL(camera.get());
  Node camera_node;

  camera_node["label"] = camera->getLabel();
  if(camera->getId().isValid()) {
    camera_node["id"] = camera->getId().hexString();
  }
  camera_node["line-delay-nanoseconds"] = camera->getLineDelayNanoSeconds();
  camera_node["image_height"] = camera->imageHeight();
  camera_node["image_width"] = camera->imageWidth();
  switch(camera->getType()) {
    case aslam::Camera::Type::kPinhole:
      camera_node["type"] = "pinhole";
      break;
    case aslam::Camera::Type::kUnifiedProjection:
      camera_node["type"] = "unified-projection";
      break;
    default:
      LOG(FATAL) << "Unknown camera model: "
        << static_cast<std::underlying_type<aslam::Camera::Type>::type>(camera->getType());
  }
  camera_node["intrinsics"] = camera->getParameters();

  const aslam::Distortion* distortion = camera->getDistortion();
  if(distortion && distortion->getType() != aslam::Distortion::Type::kNoDistortion) {
    Node distortion_node;
    switch(distortion->getType()) {
      case aslam::Distortion::Type::kEquidistant:
        distortion_node["type"] = "equidistant";
        break;
      case aslam::Distortion::Type::kFisheye:
        distortion_node["type"] = "fisheye";
        break;
      case aslam::Distortion::Type::kRadTan:
        distortion_node["type"] = "radial-tangential";
        break;
      default:
        LOG(FATAL) << "Unknown distortion model: "
          << static_cast<std::underlying_type<aslam::Distortion::Type>::type>(
              distortion->getType());
    }
    distortion_node["parameters"] = distortion->getParameters();
    camera_node["distortion"] = distortion_node;
  }
  return camera_node;
}


}  // namespace YAML

