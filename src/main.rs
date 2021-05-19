#[macro_use]
extern crate serde_derive;

extern crate libc;
use libc::{c_char, c_int};
use std::ffi::{CStr, CString};

#[link(name = adm_engine)]
extern "C" {
    fn renderAdmContent(input: *mut *const c_char,
                        destination: *mut *const c_char,
                        element_gains_cstr: *mut *const c_char,
                        element_id_to_render_cstr: *mut *const c_char,
                        output_message: *mut *const c_char) -> c_int;
}

use mcai_worker_sdk::{job::JobResult, prelude::*, MessageEvent};

pub mod built_info {
  include!(concat!(env!("OUT_DIR"), "/built.rs"));
}

#[derive(Debug, Default)]
struct AdmEngineEvent {}

#[derive(Debug, Clone, Deserialize, JsonSchema)]
pub struct WorkerParameters {
  /// # Element ID
  ///
  element_id: String,
  /// # Gain Mapping
  ///
  gain_mapping: Vec<String>,
  destination_path: String,
  source_path: String,
}

impl MessageEvent<WorkerParameters> for AdmEngineEvent {
  fn get_name(&self) -> String {
    "ADM Engine worker".to_string()
  }

  fn get_short_description(&self) -> String {
    "Worker to rebalance audio loudness".to_string()
  }

  fn get_description(&self) -> String {
    r#"This worker rebalances audio loudness."#.to_string()
  }

  fn get_version(&self) -> Version {
    Version::parse(built_info::PKG_VERSION).expect("unable to locate Package version")
  }

  fn process(
    &self,
    channel: Option<McaiChannel>,
    parameters: WorkerParameters,
    job_result: JobResult,
  ) -> Result<JobResult> {
    info!(target: &job_result.get_str_job_id(), "Start ADM Engine Job");

    let source_path = CString::new(parameters.source_path).unwrap();
    let source_path_ptr: *const c_char = source_path.as_ptr();

    let destination_path = CString::new(parameters.destination_path).unwrap();
    let destination_path_ptr: *const c_char = destination_path.as_ptr();

    // TODO: get whole array
    let gain_mapping = CString::new(parameters.gain_mapping[0]).unwrap();
    let gain_mapping_ptr: *const c_char = gain_mapping.as_ptr();

    let element_id = CString::new(parameters.element_id).unwrap();
    let element_id_ptr: *const c_char = element_id.as_ptr();

    let mut output_message = std::ptr::null();

    if renderAdmContent(&mut source_path_ptr,
                        &mut destination_path_ptr,
                        &mut gain_mapping_ptr,
                        &mut element_id_ptr,
                        &mut output_message) != 0 {
                      let message = unsafe { CStr::from_ptr(output_message).to_str().unwrap().to_owned() };
                      error!(target: &job_result.get_str_job_id(), "{}", message);
                      return Ok(job_result.with_status(JobStatus::Error)
                                          .with_message(&message))
                     }

    Ok(job_result.with_status(JobStatus::Completed))
  }
}

fn main() {
  let worker = AdmEngineEvent::default();
  start_worker(worker);
}
