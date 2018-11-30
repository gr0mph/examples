// Copyright 2018 Open Source Robotics Foundation, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <inttypes.h>
#include <memory>
#include "example_interfaces/action/fibonacci.hpp"
#include "rclcpp/rclcpp.hpp"
// TODO(jacobperron): Remove this once it is included as part of 'rclcpp.hpp'
#include "rclcpp_action/rclcpp_action.hpp"

using Fibonacci = example_interfaces::action::Fibonacci;


int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  auto node = rclcpp::Node::make_shared("minimal_action_client");
  auto action_client = rclcpp_action::create_client<Fibonacci>(node, "fibonacci");

  // Populate a goal
  auto goal_msg = Fibonacci::Goal();
  goal_msg.order = 10;

  RCLCPP_INFO(node->get_logger(), "Sending goal");
  // Ask server to achieve some goal and wait until it's accepted
  auto goal_handle_future = action_client->async_send_goal(goal_msg);
  if (rclcpp::spin_until_future_complete(node, goal_handle_future) !=
    rclcpp::executor::FutureReturnCode::SUCCESS)
  {
    RCLCPP_ERROR(node->get_logger(), "send goal call failed :(");
    return 1;
  }

  // TODO(sloretz) how to check if the goal was rejected?

  // Wait for the server to be done with the goal
  auto goal_handle = goal_handle_future.get();

  auto result_future = goal_handle->async_result();

  RCLCPP_INFO(node->get_logger(), "Waiting for result");
  if (rclcpp::spin_until_future_complete(node, result_future) !=
    rclcpp::executor::FutureReturnCode::SUCCESS)
  {
    RCLCPP_ERROR(node->get_logger(), "get result call failed :(");
    return 1;
  }

  // TODO(sloretz) how to check status of result?
  auto result = result_future.get();

  RCLCPP_INFO(node->get_logger(), "result received");
  for (auto number : result.sequence)
  {
    RCLCPP_INFO(node->get_logger(), "%" PRId64, number);
  }

  rclcpp::shutdown();
  return 0;
}
